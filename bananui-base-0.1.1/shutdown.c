/*
	App list app (homescreen)
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
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>

#define MAX_RECV_LINE_SIZE 1024
#ifdef HAVE_DEBUG
# define DEBUG(s) printf("DEBUG: %s:%d (in %s): %s\n", __FILE__, __LINE__, \
	__FUNCTION__, s)
#else
# define DEBUG(s)
#endif
static const char string [] =
	"lbc Power\n"		/* ID: 0 */
	"tit Power Options\n"
	"skc SELECT\n"
	"btn Power Off\n"	/* ID: 1 */
	"btn Reboot\n"		/* ID: 2 */
	"rfr \n";

void processResponse(int fd, const char *response)
{
	DEBUG(response);
	if(0 == strncmp(response, "clk ", 4)){
		int id;
		char *end;
		id = strtol(response+4, &end, 10);
		if(end == response+4) return;
		if(fork() == 0){
			const char *path = (id == 2 ? "/sbin/reboot" :
				"/sbin/poweroff");
			execl(path, path, NULL);
			perror(path);
			exit(140);
		}
	}
	else if(0 == strncmp(response, "exi ", 4)){
		exit(0);
	}
}

int main()
{
	struct sockaddr_un addr = {AF_UNIX, "/tmp/bananui.sock"};
	int fd, bufindex = 0;
	char recvbuf[MAX_RECV_LINE_SIZE];
	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(fd < 0){
		perror("socket");
		return 1;
	}
	if(connect(fd, (struct sockaddr*) &addr, sizeof(addr)) < 0){
		perror("Failed to bind to /tmp/bananui.sock");
		return 1;
	}
	if(write(fd, string, sizeof(string)-1) < 0){
		perror("write");
		return 1;
	}
	while(read(fd, recvbuf+bufindex, 1) > 0){
		if(recvbuf[bufindex] == '\n'){
			recvbuf[bufindex] = 0;
			bufindex = 0;
			processResponse(fd, recvbuf);
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
