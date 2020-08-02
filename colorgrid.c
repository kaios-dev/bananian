#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
	"tit Color Grid\n"
	"sty 255 255 255 0 0 0\n"
	"cbx \nlbl n\nlbl n\nlbl n\nlbl n\nlbl n\nlbl n\nlbl n\ncbe \n"
	"cbx \nlbl n\nlbl n\nlbl n\nlbl n\nlbl n\nlbl n\nlbl n\ncbe \n"
	"cbx \nlbl n\nlbl n\nlbl n\nlbl n\nlbl n\nlbl n\nlbl n\ncbe \n"
	"sty 255 255 0 0 0 255\n"
	"cbx \nlbl n\nlbl n\nlbl n\nlbl n\nlbl n\nlbl n\nlbl n\ncbe \n"
	"sty 255 255 255 0 0 0\n"
	"cbx \nlbl n\nlbl n\nlbl n\nlbl n\nlbl n\nlbl n\nlbl n\ncbe \n"
	"cbx \nlbl n\nlbl n\nlbl n\nlbl n\nlbl n\nlbl n\nlbl n\ncbe \n"
	"cbx \nlbl n\nlbl n\nlbl n\nlbl n\nlbl n\nlbl n\nlbl n\ncbe \n"
	"sty 255 255 0 0 0 0\n"
	"rep 25 lbl k\n"
	"sty 255 255 255 0 0 0\n"
	"rfr \n";

void processResponse(int fd, const char *response)
{
	DEBUG(response);
	if(0 == strncmp(response, "exi ", 4)){
		exit(0);
	}
}

int main(){
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
