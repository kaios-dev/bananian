#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>

#define MAX_RECV_LINE_SIZE 1024
#define NAME_SIZE 256
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

static const char string [] =
	"lbc Apps\n"
	"tit Browse Apps\n"
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

void wrap_applist(char ***appnames, char ***execpaths,
	struct applist_item *tail, int lsize)
{
	*appnames = malloc(lsize * sizeof(char *));
	*execpaths = malloc(lsize * sizeof(char *));
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

void browse(int fd, const struct config *cfg)
{
	int i;
	for(i = 0; i < cfg->numapps; i++){
		write(fd, "btn ", 4);
		DEBUG(cfg->app_names[i]);
		write(fd, cfg->app_names[i], strlen(cfg->app_names[i]));
		write(fd, "\n", 1);
	}
	write(fd, "rfr \n", 5);
}

void processResponse(int fd, struct config *cfg, const char *response)
{
	DEBUG(response);
	waitpid(-1, NULL, WNOHANG);
	if(0 == strncmp(response, "clk ", 4)){
		int id;
		char *end;
		id = strtol(response+4, &end, 10);
		if(end == response+4) return;
		if(fork() == 0){
			execl(cfg->exec_paths[id-1], cfg->exec_paths[id-1],
				NULL);
			perror(cfg->exec_paths[id-1]);
			exit(140);
		}
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
	if(connect(fd, (struct sockaddr*) &addr, sizeof(addr)) < 0){
		perror("Failed to bind to /tmp/bananui.sock");
		return 1;
	}
	if(write(fd, string, sizeof(string)-1) < 0){
		perror("write");
		return 1;
	}
	parse_config("/usr/share/bananui.apps", &cfg);
	browse(fd, &cfg);
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
