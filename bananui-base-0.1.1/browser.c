/*
	File browser for Bananian
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
#include <errno.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>

#define MAX_RECV_LINE_SIZE 1024
#define NAME_SIZE 1024
#define PATH_SIZE 256
#ifdef HAVE_DEBUG
# define DEBUG(s) printf("DEBUG: %s:%d (in %s): %s\n", __FILE__, __LINE__, \
	__FUNCTION__, s)
#else
# define DEBUG(s)
#endif
struct config {
	int numapps;
	char **exec_paths;
	char **app_names;
};
struct applist_item {
	struct applist_item *prev;
	char app_name[NAME_SIZE];
	char exec_path[PATH_SIZE];
};
struct filelist_item {
	struct filelist_item *prev;
	char file_name[NAME_SIZE];
};
char **filenames = NULL+1; /* Not null */
int numfiles;

static const char initstring [] =
	"clr \n"
	"lbc Files\n"
	"tit File browser\n"
	"skc OPEN\n";


struct applist_item *create_new_tail(struct applist_item *prevtail)
{
	struct applist_item *tail;
	DEBUG("entered");
	tail = malloc(sizeof(struct applist_item));
	if(!tail) return tail; /* NULL */
	tail->prev = prevtail;
	DEBUG("created tail");
	return tail;
}

struct filelist_item *create_new_file_tail(struct filelist_item *prevtail)
{
	struct filelist_item *tail;
	DEBUG("entered");
	tail = malloc(sizeof(struct filelist_item));
	if(!tail) return tail; /* NULL */
	tail->prev = prevtail;
	DEBUG("created tail");
	return tail;
}

void wrap_applist(char ***appnames, char ***execpaths,
	struct applist_item *tail, int lsize)
{
	*appnames = malloc(lsize * sizeof(char **));
	*execpaths = malloc(lsize * sizeof(char **));
	while(lsize){
		DEBUG("Copying appname");
		(*appnames)[lsize-1] = malloc(NAME_SIZE);
		strncpy((*appnames)[lsize-1], tail->app_name, NAME_SIZE-1);
		DEBUG("Copying execpath");
		(*execpaths)[lsize-1] = malloc(PATH_SIZE);
		strncpy((*execpaths)[lsize-1], tail->exec_path,
			PATH_SIZE-1);
		DEBUG((*appnames)[lsize-1]);
		DEBUG("Copy done");
		lsize--;
		tail = tail->prev;
	}
	DEBUG("Wrapped");
}
void wrap_filelist(struct filelist_item *tail)
{
	int lsize = numfiles;
	filenames = malloc(lsize * sizeof(char *));
	while(lsize){
		DEBUG("Copying filename");
		filenames[lsize-1] = malloc(NAME_SIZE);
		strncpy(filenames[lsize-1], tail->file_name, NAME_SIZE-1);
		DEBUG(filenames[lsize-1]);
		DEBUG("Copy done");
		lsize--;
		tail = tail->prev;
	}
	DEBUG("Wrapped");
}

void parse_config(const char *config_file_name, struct config *cfg)
{
	int cfd, lsize = 0;
	struct applist_item *tail = NULL, *oldtail = NULL;
	cfd = open(config_file_name, O_RDONLY);
	if(cfd < 0){
		perror(config_file_name);
		exit(2);
	}
	while((tail = create_new_tail(tail))){
		int i = -1;
		do {
			if(!read(cfd, tail->app_name + (++i), 1)){
				tail = tail->prev;
				goto out;
			}
		} while(tail->app_name[i] != '=');
		DEBUG("Read appname");
		tail->app_name[i] = 0;
		DEBUG(tail->app_name);
		i = -1;
		do {
			if(!read(cfd, tail->exec_path + (++i), 1)){
				tail = tail->prev;
				goto out;
			}
		} while(tail->exec_path[i] != '\n');
		DEBUG("Read execpath");
		tail->exec_path[i] = 0;
		DEBUG(tail->exec_path);
		lsize++;
		oldtail = tail;
	}
	tail = oldtail;
out:
	wrap_applist(&cfg->app_names, &cfg->exec_paths, tail, lsize);
	cfg->numapps = lsize;
}

void browse(int fd, const struct config *cfg, const char *dirname)
{
	int pipefd[2];
	char cwdbuf[PATH_MAX];
	struct filelist_item *tail = NULL, *oldtail = NULL;
	numfiles = 0;
	if(chdir(dirname) < 0){
		char *errname;
		errname = strerror(errno);
		write(fd, "lbl Failed to open directory: ", 30);
		write(fd, errname, strlen(errname));
		write(fd, "\nrfr \n", 6);
		filenames = NULL; /* show that something is wrong */
		return;
	}
	getcwd(cwdbuf, PATH_MAX);
	write(fd, "lbc ", 4);
	write(fd, cwdbuf, strlen(cwdbuf));
	write(fd, "\n", 1);
	write(fd, "tit ", 4);
	write(fd, cwdbuf, strlen(cwdbuf));
	write(fd, "\n", 1);
	if(pipe(pipefd) < 0){
		perror("pipe");
		exit(3);
	}
	if(fork() == 0){
		close(1);
		dup2(pipefd[1], 1);
		close(pipefd[0]);
		close(pipefd[1]);
		execl("/bin/ls", "ls", NULL);
		perror("/bin/ls");
		exit(140);
	}
	close(pipefd[1]);
	while((tail = create_new_file_tail(tail))){
		char filename[NAME_SIZE], ch;
		int fni = -1;
		do {
			if(read(pipefd[0], filename + ++fni, 1) <= 0)
				goto end;
		} while(filename[fni] != '\n' && fni < NAME_SIZE-1);
		if(fni == NAME_SIZE-1) do {
			if(read(pipefd[0], &ch, 1) <= 0) goto end;
		} while(ch != '\n');
		filename[fni] = 0;
		write(fd, "btn ", 4);
		write(fd, filename, strlen(filename));
		write(fd, "\n", 1);
		strcpy(tail->file_name, filename);
		numfiles++;
		oldtail = tail;
	}
	close(pipefd[0]);
end:
	if(tail) free(tail);
	tail = oldtail;
	wait(NULL);
	wrap_filelist(tail);
	write(fd, "rfr \n", 5);
}

void processResponse(int fd, struct config *cfg, const char *response)
{
	DEBUG(response);
	if(0 == strncmp(response, "clk ", 4)){
		int id;
		char *end;
		struct stat fileinfo;
		id = strtol(response+4, &end, 10);
		if(end == response+4) return;
		if(stat(filenames[id-2], &fileinfo) < 0){
			DEBUG(filenames[id-2]);
			perror("stat");
			return;
		}
		if((fileinfo.st_mode & S_IFMT) == S_IFDIR){
			write(fd, initstring, sizeof(initstring)-1);
			browse(fd, cfg, filenames[id-2]);
		}
/*		if(fork() == 0){
			execl(cfg->exec_paths[id-1], cfg->exec_paths[id-1],
				NULL);
			perror(cfg->exec_paths[id-1]);
			exit(140);
		}*/
	}
	else if(0 == strncmp(response, "exi ", 4)){
		char cwdbuf[2];
		int i, waserror = 0;
		/* If the filenames array is NULL, something went wrong. */
		if(!filenames) waserror = 1;
		for(i = 0; i < numfiles; i++){
			free(filenames[i]);
		}
		free(filenames);
		/* Exit if the path is only one character long */
		if(getcwd(cwdbuf, 2) >= 0 && cwdbuf[1] == 0 &&
			cwdbuf[0] == '/' &&!waserror)
		{
			exit(0);
		}
		write(fd, initstring, sizeof(initstring)-1);
		browse(fd, cfg, waserror ? "." : "..");
	}
}

int main()
{
	struct sockaddr_un addr = {AF_UNIX, "/tmp/bananui.sock"};
	int fd, bufindex = 0;
	char recvbuf[MAX_RECV_LINE_SIZE];
	struct config cfg;
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
	if(write(fd, initstring, sizeof(initstring)-1) < 0){
		perror("write");
		return 1;
	}
	parse_config("/dev/null", &cfg);
	browse(fd, &cfg, getenv("HOME"));
	while(read(fd, recvbuf+bufindex, 1) > 0){
		if(recvbuf[bufindex] == '\n'){
			recvbuf[bufindex] = 0;
			bufindex = 0;
			processResponse(fd, &cfg, recvbuf);
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
