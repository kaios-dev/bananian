/*
	Login screen
        Copyright (C) 2020 Affe Null <affenull2345@gmail.com>

        This program is free software: you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation, either version 3 of the License, or
        any later version.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <pwd.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>

#define MAX_RECV_LINE_SIZE 1024
#ifdef HAVE_DEBUG
# define DEBUG(s) printf("DEBUG: %s:%d (in %s): %s\n", __FILE__, __LINE__, \
	__FUNCTION__, s)
#else
# define DEBUG(s)
#endif

enum restart_type { NORESTART, SOFTRESTART, FULLRESTART };

static const char string [] =
	"clr \n"			/* For restart */
	"skl Power\n"
	"skc LOG IN\n"
	"lbc Bananian v" VERSION "\n"	/* ID: 0 */
	"tit Bananian Login\n"
	"lbl \n"			/* ID: 1 */
	"lbl \n"			/* ID: 2 */
	"lbc Log in\n"			/* ID: 3 */
	"lbl \n"			/* ID: 4 */
	"lbl Username\n"		/* ID: 5 */
	"inp \n"			/* ID: 6 */
	"rfr \n";

static const char getusername [] = "get 6\n";
static const char getother [] = "get 1\n";

int readLine(int fd, char *dest, size_t maxsize)
{
	char ch;
	int index = 0;
	while(read(fd, &ch, 1) >= 0 && ch != '\n'){
		if(ch == '\r') continue;
		if(dest) dest[index] = ch;
		index++;
		if(index == maxsize-1) break;
	}
	if(dest) dest[index] = '\0';
}

int converse(int n, const struct pam_message **msg,
	struct pam_response **resp, void *mydata)
{
	int fd = *((int*)mydata);
	int i;
	for(i = 0; i < n; i++){
		if(msg[i]->msg_style == PAM_ERROR_MSG ||
			msg[i]->msg_style == PAM_TEXT_INFO)
		{
			write(fd, "lbl Error: ", 11);
			write(fd, msg[i]->msg, strlen(msg[i]->msg));
			write(fd, "\nrfr \n", 6);
		}
		else if(msg[i]->msg_style == PAM_PROMPT_ECHO_OFF ||
			msg[i]->msg_style == PAM_PROMPT_ECHO_ON)
		{
			char recvbuf[MAX_RECV_LINE_SIZE];
			write(fd, "clr \nlbl ", 9);
			write(fd, msg[i]->msg, strlen(msg[i]->msg));
			if(msg[i]->msg_style == PAM_PROMPT_ECHO_OFF)
				write(fd, "\ninp p\nrfr \n",/* Password input */
					12);
			else
				write(fd, "\ninp \nrfr \n",/* Normal input */
					11);
			do {
				readLine(fd, recvbuf, MAX_RECV_LINE_SIZE);
			} while(0 != strncmp(recvbuf, "clk ", 4));
			write(fd, getother, strlen(getother));
			do {
				readLine(fd, recvbuf, MAX_RECV_LINE_SIZE);
			} while(0 != strncmp(recvbuf, "set ", 4));
			resp[i] = malloc(sizeof(struct pam_response));
			resp[i]->resp = strdup(recvbuf + 4);
			resp[i]->resp_retcode = 0;
		}
	}
	return PAM_SUCCESS;
}

enum restart_type tryLogin(int fd)
{
	char set_username[MAX_RECV_LINE_SIZE];
	const char *username;
	char *envpath;
	int stat, pid;
	pam_handle_t *pamhan;
	struct pam_conv conv;
	struct passwd *passent;
	envpath = getenv("PATH");
	conv.conv = converse;
	conv.appdata_ptr = &fd;
	write(fd, getusername, sizeof(getusername)-1);
	do {
		readLine(fd, set_username, MAX_RECV_LINE_SIZE);
	} while(0 != strncmp(set_username, "set ", 4));
	pam_start("bananui-login", set_username+4, &conv, &pamhan);
	write(fd, "skl \n", 5);
	stat = pam_authenticate(pamhan, 0);
	if(stat != PAM_SUCCESS){
		const char *errstr = pam_strerror(pamhan, stat);
		write(fd, "lbl ", 4);
		write(fd, errstr, strlen(errstr));
		write(fd, "\nrfr \n", 6);
		sleep(3);
		return SOFTRESTART;
	}
	pam_get_user(pamhan, &username, "Username: ");
	passent = getpwnam(username);
	pam_end(pamhan, stat);
	close(fd);
	if((pid = fork()) == 0){
		setuid(passent->pw_uid);
		setgid(passent->pw_gid);
		setsid();
		clearenv();
		setenv("HOME", passent->pw_dir, 0);
		setenv("SHELL", passent->pw_shell, 0);
		if(envpath)
			setenv("PATH", envpath, 0);
		else
			setenv("PATH", "/usr/local/bin:/usr/bin:/bin", 0);
		if(chdir(passent->pw_dir) < 0){
			perror(passent->pw_dir);
			chdir("/");
		}
		execl("/usr/bin/mainclient", "mainclient", NULL);
		perror("/usr/bin/mainclient");
	}
	waitpid(pid, NULL, 0);
	return FULLRESTART;
}

enum restart_type processResponse(int fd, const char *response)
{
	DEBUG(response);
	if(0 == strncmp(response, "clk ", 4)){
		return tryLogin(fd);
	}
	else if(0 == strcmp(response, "kdn 139")){
		if(fork() == 0){
			execl("/usr/bin/bananui-shutdown", "bananui-shutdown",
				NULL);
			perror("/usr/bin/bananui-shutdown");
		}
	}
	return NORESTART;
}

int main(){
	struct sockaddr_un addr = {AF_UNIX, "/tmp/bananui.sock"};
	int fd, bufindex = 0;
	char recvbuf[MAX_RECV_LINE_SIZE];
fullrestart:
	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(fd < 0){
		perror("socket");
		return 1;
	}
	fcntl(fd, F_SETFD, FD_CLOEXEC);
	if(connect(fd, (struct sockaddr*) &addr, sizeof(addr)) < 0){
		perror("Failed to bind to /tmp/bananui.sock");
		return 1;
	}
restart:
	if(write(fd, string, sizeof(string)-1) < 0){
		perror("write");
		return 1;
	}
	while(read(fd, recvbuf+bufindex, 1) > 0){
		if(recvbuf[bufindex] == '\n'){
			int res;
			recvbuf[bufindex] = 0;
			bufindex = 0;
			res = processResponse(fd, recvbuf);
			if(res == SOFTRESTART) goto restart;
			if(res == FULLRESTART) goto fullrestart;
		}
		else if(bufindex == MAX_RECV_LINE_SIZE - 1){
			char ch;
			recvbuf[bufindex] = 0;
			while(read(fd, &ch, 1) && ch != '\n')
				;
		}
		else bufindex++;
	}
	return 0;
}
