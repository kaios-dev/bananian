/*
	Fake login screen with a bitmap demo
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
#include <sys/socket.h>
#include <sys/un.h>

#define MAX_RECV_LINE_SIZE 1024
#ifdef HAVE_DEBUG
# define DEBUG(s) printf("DEBUG: %s:%d (in %s): %s\n", __FILE__, __LINE__, \
	__FUNCTION__, s)
#else
# define DEBUG(s)
#endif

static const char preimg [] = "cbx \niml \n\x14\x0b";
static const unsigned int img [11][20] = {
	{
		0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00,
		0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00,
		0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00,
		0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00
	},
	{
		0xff00ff00, 0x00000000, 0xff0000ff, 0xff0000ff, 0xff0000ff,
		0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff,
		0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff,
		0xff0000ff, 0xff0000ff, 0xff0000ff, 0x00000000, 0xff00ff00
	},
	{
		0xff00ff00, 0x00000000, 0xff0000ff, 0x00000000, 0xffff0000,
		0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
		0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
		0xffff0000, 0x00000000, 0xff0000ff, 0x00000000, 0xff00ff00
	},
	{
		0xff00ff00, 0x00000000, 0xff0000ff, 0x00000000, 0xffff0000,
		0x00000000, 0xffff00ff, 0xffff00ff, 0xffff00ff, 0xffff00ff,
		0xffff00ff, 0xffff00ff, 0xffff00ff, 0xffff00ff, 0x00000000,
		0xffff0000, 0x00000000, 0xff0000ff, 0x00000000, 0xff00ff00
	},
	{
		0xff00ff00, 0x00000000, 0xff0000ff, 0x00000000, 0xffff0000,
		0x00000000, 0xffff00ff, 0x00000000, 0xffffffff, 0xffffffff,
		0xffffffff, 0xffffffff, 0x00000000, 0xffff00ff, 0x00000000,
		0xffff0000, 0x00000000, 0xff0000ff, 0x00000000, 0xff00ff00
	},
	{
		0xff00ff00, 0x00000000, 0xff0000ff, 0x00000000, 0xffff0000,
		0x00000000, 0xffff00ff, 0x00000000, 0xffffffff, 0x00000000,
		0xffffff00, 0x00000000, 0x00000000, 0xffff00ff, 0x00000000,
		0xffff0000, 0x00000000, 0xff0000ff, 0x00000000, 0xff00ff00
	},
	{
		0xff00ff00, 0x00000000, 0xff0000ff, 0x00000000, 0xffff0000,
		0x00000000, 0xffff00ff, 0x00000000, 0xffffffff, 0xffffffff,
		0xffffffff, 0xffffffff, 0x00000000, 0xffff00ff, 0x00000000,
		0xffff0000, 0x00000000, 0xff0000ff, 0x00000000, 0xff00ff00
	},
	{
		0xff00ff00, 0x00000000, 0xff0000ff, 0x00000000, 0xffff0000,
		0x00000000, 0xffff00ff, 0xffff00ff, 0xffff00ff, 0xffff00ff,
		0xffff00ff, 0xffff00ff, 0xffff00ff, 0xffff00ff, 0x00000000,
		0xffff0000, 0x00000000, 0xff0000ff, 0x00000000, 0xff00ff00
	},
	{
		0xff00ff00, 0x00000000, 0xff0000ff, 0x00000000, 0xffff0000,
		0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
		0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
		0xffff0000, 0x00000000, 0xff0000ff, 0x00000000, 0xff00ff00
	},
	{
		0xff00ff00, 0x00000000, 0xff0000ff, 0xff0000ff, 0xff0000ff,
		0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff,
		0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff,
		0xff0000ff, 0xff0000ff, 0xff0000ff, 0x00000000, 0xff00ff00
	},
	{
		0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00,
		0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00,
		0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00,
		0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00
	}
};
static const char string [] =
	"lbc Bananian v" VERSION "\ncbe \n"
	"tit Bananian Login\n"
	"lbl \n"
	"lbl \n"
	"lbc Log in\n"
	"lbl \n"
	"lbl Username\n"
	"inp \n"
	"lbl \n"
	"lbl Password \n"
	"inp \n"
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
	write(fd, preimg, sizeof(preimg)-1);
	write(fd, img, sizeof(img));
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
