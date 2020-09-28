/*
	The BananUI server. Handles commands from apps.
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
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "ui.h"
#define MAX_SOCK_FDS 128
#define MAX_WAITING_CLIENTS 5
#define MAX_COMMAND_LENGTH 1024

enum bintype { BINARY_NONE = 0, BINARY_IMAGE = 1 };

struct current_style {
	int fgr, fgg, fgb;
	int bgr, bgg, bgb;
};

struct mydata {
	struct uiinfo *uiinf;
	struct ui_window *win;
	struct ui_window* windows[MAX_SOCK_FDS];
	struct current_style* styles[MAX_SOCK_FDS];
	int sockfds[MAX_SOCK_FDS], nsockfds;
	int bufindex[MAX_SOCK_FDS];
	enum bintype isbinary[MAX_SOCK_FDS];
	char buffers[MAX_SOCK_FDS][MAX_COMMAND_LENGTH];
	struct ui_widget* cbx[MAX_SOCK_FDS];
	struct ui_widget* img[MAX_SOCK_FDS];
};

static void initStyle(struct current_style *style){
	style->fgr = style->fgg = style->fgb = 0;
	style->bgr = style->bgg = style->bgb = 0xA0;
}

static void initWidgetStyle(struct mydata *mydat, struct ui_style *style,
	int i)
{
	struct current_style *cst = mydat->styles[i];
	style->fgcolor = from_rgb(cst->fgr, cst->fgg, cst->fgb);
	style->bgcolor = from_rgb(cst->bgr, cst->bgg, cst->bgb);
}

static int doLabel(struct mydata *mydat, struct ui_window *win,
	struct ui_widget *widget, const char *command)
{
	widget->type = UI_LABEL;
	switch(command[2]){
		case 'l':
			widget->textalign = UI_TA_LEFT;
			break;
		case 'c':
			widget->textalign = UI_TA_CENTER;
			break;
		case 'r':
			widget->textalign = UI_TA_RIGHT;
			break;
		default:
			return 0;
	}
	if(command[3] != ' ') return 0;
	widget->data = malloc(strlen(command+3));
	widget->freedata = 1;
	strcpy((char*) widget->data, command+4);
	widget->rightend = mydat->uiinf->fbinf->vinfo->xres;
	return 1;
}
static void saveByte(struct mydata *mydat, int clientid, char byte)
{
	unsigned char *data;
	data = mydat->img[clientid]->data;
	if(!data){
		data = malloc(2);
		data[0] = byte;
		data[1] = 0;
		mydat->bufindex[clientid] = 0;
	}
	else if(!data[1]){
		data = realloc(data, byte * data[0] *
			sizeof(unsigned int) + 2);
		mydat->img[clientid]->freedata = 1;
		data[1] = byte;
	}
	else {
		data[2 + mydat->bufindex[clientid]] = byte;
		mydat->bufindex[clientid]++;
		if(mydat->bufindex[clientid] == data[0] * data[1] *
			sizeof(unsigned int))
		{
			mydat->isbinary[clientid] = 0;
			mydat->img[clientid] = NULL;
			mydat->bufindex[clientid] = 0;
			return;
		}
	}
	mydat->img[clientid]->data = data;
}
static int doImage(struct mydata *mydat, struct ui_window *win,
	struct ui_widget *widget, const char *command, int clientid)
{
	widget->type = UI_IMAGE;
	switch(command[2]){
		case 'l':
			widget->textalign = UI_TA_LEFT;
			break;
		case 'c':
			widget->textalign = UI_TA_CENTER;
			break;
		case 'r':
			widget->textalign = UI_TA_RIGHT;
			break;
		default:
			return 0;
	}
	if(command[3] != ' ') return 0;
	widget->data = NULL;
	widget->freedata = 0;
	mydat->isbinary[clientid] = BINARY_IMAGE;
	mydat->img[clientid] = widget;
	widget->rightend = mydat->uiinf->fbinf->vinfo->xres;
	return 1;
}
static int doButton(struct mydata *mydat, struct ui_window *win,
	struct ui_widget *widget, const char *command)
{
	widget->type = UI_BUTTON;
	widget->data = malloc(strlen(command+3));
	widget->freedata = 1;
	strcpy((char*) widget->data, command+4);
	widget->rightend = mydat->uiinf->fbinf->vinfo->xres;
	return 1;
}

static int doInput(struct mydata *mydat, struct ui_window *win,
	struct ui_widget *widget, const char *command)
{
	widget->type = UI_INPUT;
	widget->data = malloc(258);
	widget->freedata = 1;
	switch(command[4]){
		case 'p':
			widget->inputtype = UI_INP_PASSWORD;
			break;
		default:
			widget->inputtype = UI_INP_NORMAL;
	}
	if(command[4] && command[5] == ' '){
		strncpy(((char*)widget->data) + 2, command+6, 256);
	}
	else {
		((char*)widget->data)[2] = 0;
	}
	((char*)widget->data)[0] = 0;
	((char*)widget->data)[1] = 0;
	widget->rightend = mydat->uiinf->fbinf->vinfo->xres;
	return 1;
}
static void returnContents(struct mydata *mydat, int fd,
	struct ui_widget *widget)
{
	write(fd, "set ", 4); /* So it won't conflict with events */
	switch(widget->type){
		case UI_IMAGE:
		case UI_LABEL:
		case UI_BUTTON:
		case UI_FOBUTTON:
			write(fd, widget->data,
				strlen((char*)widget->data));
			break;
		case UI_INPUT:
		case UI_FOINPUT:
			write(fd, ((char*)widget->data)+2,
				strlen(((char*)widget->data)+2));
			break;
		case UI_CBX:
			write(fd, "CBX", 3);
			break;
	}
}
static void serverCommand(struct mydata *mydat, int i, const char *command)
{
	if(0 == strncmp(command, "btn ", 4)){
		struct ui_widget *widget;
		widget = malloc(sizeof(struct ui_widget));
		initWidgetStyle(mydat, &widget->style, i);
		if(!doButton(mydat, mydat->windows[i], widget, command)){
			free(widget);
			return;
		}
		if(mydat->cbx[i]){
			addWidgetToCbx(mydat->uiinf, mydat->windows[i],
				mydat->cbx[i], widget, 1);
			mydat->cbx[i]->focusable = 1;
		}
		else
			addWidget(mydat->uiinf, mydat->windows[i], widget,
				1);
	}
	else if(0 == strncmp(command, "inp ", 4)){
		struct ui_widget *widget;
		widget = malloc(sizeof(struct ui_widget));
		initWidgetStyle(mydat, &widget->style, i);
		if(!doInput(mydat, mydat->windows[i], widget, command)){
			free(widget);
			return;
		}
		if(mydat->cbx[i]){
			addWidgetToCbx(mydat->uiinf, mydat->windows[i],
				mydat->cbx[i], widget, 1);
			mydat->cbx[i]->focusable = 1;
		}
		else
			addWidget(mydat->uiinf, mydat->windows[i],
				widget, 1);
	}
	else if(0 == strncmp(command, "lb", 2)){
		struct ui_widget *widget;
		widget = malloc(sizeof(struct ui_widget));
		initWidgetStyle(mydat, &widget->style, i);
		if(!doLabel(mydat, mydat->windows[i], widget, command)){
			free(widget);
			return;
		}
		if(mydat->cbx[i])
			addWidgetToCbx(mydat->uiinf, mydat->windows[i],
				mydat->cbx[i], widget, 0);
		else
		addWidget(mydat->uiinf, mydat->windows[i],
			widget, 0);
	}
	else if(0 == strncmp(command, "im", 2)){
		struct ui_widget *widget;
		widget = malloc(sizeof(struct ui_widget));
		initWidgetStyle(mydat, &widget->style, i);
		if(!doImage(mydat, mydat->windows[i], widget, command, i)){
			free(widget);
			return;
		}
		if(mydat->cbx[i])
			addWidgetToCbx(mydat->uiinf, mydat->windows[i],
				mydat->cbx[i], widget, 0);
		else
			addWidget(mydat->uiinf, mydat->windows[i],
				widget, 0);
	}
	else if(0 == strncmp(command, "cbx ", 4)){
		struct ui_cbx_info *cbxi;
		if(mydat->cbx[i]) return;
		mydat->cbx[i] = malloc(sizeof(struct ui_widget));
		mydat->cbx[i]->type = UI_CBX;
		cbxi = malloc(sizeof(struct ui_cbx_info));
		cbxi->firstfoc = cbxi->lastfoc = cbxi->first = cbxi->last =
			NULL;
		cbxi->numwidgets = 0;
		mydat->cbx[i]->data = cbxi;
		mydat->cbx[i]->freedata = 1;
		mydat->cbx[i]->focusable = 0;
	}
	else if(0 == strncmp(command, "cbe ", 4)){
		if(!mydat->cbx[i]) return;
		initWidgetStyle(mydat, &mydat->cbx[i]->style, i);
		addWidget(mydat->uiinf, mydat->windows[i],
			mydat->cbx[i], mydat->cbx[i]->focusable);
		mydat->cbx[i] = NULL;
	}
	else if(0 == strncmp(command, "rep ", 4)){
		char *other;
		int id;
		struct ui_widget *widget;
		id = strtol(command+4, &other, 10);
		if(id >= MAX_WIDGETS || id < 0 ||
			!mydat->windows[i]->widgets[id]) return;
		widget = mydat->windows[i]->widgets[id];
		if(0 == strncmp(other, " btn ", 5)){
			initWidgetStyle(mydat, &widget->style, i);
			if(!doButton(mydat, mydat->windows[i], widget,
				other+1))
				return;
			makeFocusable(mydat->uiinf, mydat->windows[i],
				widget);
		}
		if(0 == strncmp(other, " inp ", 5)){
			initWidgetStyle(mydat, &widget->style, i);
			if(!doInput(mydat, mydat->windows[i], widget,
				other+1))
				return;
			makeFocusable(mydat->uiinf, mydat->windows[i],
				widget);
		}
		else if(0 == strncmp(other, " lb", 3)){
			initWidgetStyle(mydat, &widget->style, i);
			if(!doLabel(mydat, mydat->windows[i], widget,
				other+1))
				return;
			makeUnfocusable(mydat->uiinf, mydat->windows[i],
				widget);
		}
		else if(0 == strncmp(other, " im", 3)){
			initWidgetStyle(mydat, &widget->style, i);
			if(!doImage(mydat, mydat->windows[i], widget,
				other+1, i))
				return;
			makeUnfocusable(mydat->uiinf, mydat->windows[i],
				widget);
		}
	}
	else if(0 == strcmp(command, "rfr ")){
		redrawWindow(mydat->uiinf, mydat->windows[i]);
	}
	else if(0 == strncmp(command, "get ", 4)){
		int id;
		id = strtol(command+4, NULL, 10);
		if(id < MAX_WIDGETS && id >= 0 &&
				mydat->windows[i]->widgets[id])
		{
			returnContents(mydat, mydat->sockfds[i],
				mydat->windows[i]->widgets[id]);
		}
		write(mydat->sockfds[i], "\n", 1);
	}
	else if(0 == strncmp(command, "tit ", 4)){
		strncpy(mydat->windows[i]->title, command+4, TITLE_SIZE-1);
	}
	else if(0 == strncmp(command, "sk", 2)){
		char dummy[SK_LABEL_SIZE];
		if(!command[2] || command[3] != ' ') return;
		strncpy(command[2] == 'l' ? mydat->windows[i]->current_skl
			: command[2] == 'c' ? mydat->windows[i]->current_skc
			: command[2] == 'r' ? mydat->windows[i]->current_skr
			: dummy, command+4, SK_LABEL_SIZE-1);
	}
	else if(0 == strncmp(command, "clr ", 4)){
		clearWindow(mydat->uiinf, mydat->windows[i]);
	}
	else if(0 == strncmp(command, "sty ", 4)){
		char *end;
		mydat->styles[i]->fgr = strtol(command+4, &end, 10);
		if(*end != ' ') return;
		mydat->styles[i]->fgg = strtol(end+1, &end, 10);
		if(*end != ' ') return;
		mydat->styles[i]->fgb = strtol(end+1, &end, 10);
		if(*end != ' ') return;
		mydat->styles[i]->bgr = strtol(end+1, &end, 10);
		mydat->windows[i]->bgr = mydat->styles[i]->bgr;
		if(*end != ' ') return;
		mydat->styles[i]->bgg = strtol(end+1, &end, 10);
		mydat->windows[i]->bgg = mydat->styles[i]->bgr;
		if(*end != ' ') return;
		mydat->styles[i]->bgb = strtol(end+1, &end, 10);
		mydat->windows[i]->bgb = mydat->styles[i]->bgr;
		if(*end != ' ') return;
		mydat->windows[i]->skfgr = strtol(end+1, &end, 10);
		if(*end != ' ') return;
		mydat->windows[i]->skfgg = strtol(end+1, &end, 10);
		if(*end != ' ') return;
		mydat->windows[i]->skfgb = strtol(end+1, &end, 10);
		if(*end != ' ') return;
		mydat->windows[i]->skbgr = strtol(end+1, &end, 10);
		if(*end != ' ') return;
		mydat->windows[i]->skbgg = strtol(end+1, &end, 10);
		if(*end != ' ') return;
		mydat->windows[i]->skbgb = strtol(end+1, &end, 10);
		if(*end != ' ') return;
	}
}

static void clientEventCallback(void *data, int id, enum ui_event ev)
{
	char outbuf[15];
	char *code;
	int fd;
	fd = *((int*)data);
	switch(ev){
		case UI_EVENT_CLICK:
			code = "clk";
			break;
		case UI_EVENT_KEYDOWN:
			code = "kdn";
			break;
		case UI_EVENT_KEYUP:
			code = "kup";
			break;
		case UI_EVENT_EXIT:
			code = "exi";
			break;
		default:
			return;
	}
	snprintf(outbuf, 15, "%s %d\n", code, id);
	write(fd, outbuf, strlen(outbuf));
}
static void fdCallback(void *data, int i)
{
	struct mydata *mydat = data;
	if(i == 0){
		if(mydat->nsockfds == MAX_SOCK_FDS){
			fprintf(stderr, "Too many clients\n");
			return;
		}
		mydat->sockfds[mydat->nsockfds] =
			accept(mydat->sockfds[i], NULL, NULL);
		fcntl(mydat->sockfds[mydat->nsockfds], F_SETFL, O_CLOEXEC);
		printf("Client! Id: %d\n", mydat->nsockfds);
		mydat->bufindex[mydat->nsockfds] = 0;
		mydat->isbinary[mydat->nsockfds] = BINARY_NONE;
		mydat->cbx[mydat->nsockfds] = NULL;
		mydat->img[mydat->nsockfds] = NULL;
		mydat->windows[mydat->nsockfds] =
			createWindow(mydat->uiinf);
		mydat->windows[mydat->nsockfds]->userdata =
			(void*) &mydat->sockfds[mydat->nsockfds];
		mydat->windows[mydat->nsockfds]->event =
			clientEventCallback;
		mydat->styles[mydat->nsockfds] = malloc(
			sizeof(struct current_style));
		initStyle(mydat->styles[mydat->nsockfds]);
		mydat->nsockfds++;
	}
	else {
		char ch;
		if(read(mydat->sockfds[i], &ch, 1) <= 0){
			printf("Client %d disconnected.\n", i);
			mydat->sockfds[i] = -1;
			destroyWindow(mydat->uiinf, mydat->windows[i]);
			while(mydat->sockfds[mydat->nsockfds-1] == -1)
				mydat->nsockfds--;
			return;
		}
		if(mydat->isbinary[i]) saveByte(mydat, i, ch);
		else {
			if(mydat->bufindex[i] == MAX_COMMAND_LENGTH-1){
				ch = '\n';
			}
			mydat->buffers[i][mydat->bufindex[i]] =
				ch == '\n' ? 0 : ch;
			mydat->bufindex[i]++;
			if(ch == '\n' && !mydat->isbinary[i]){
				serverCommand(mydat, i, mydat->buffers[i]);
				mydat->bufindex[i] = 0;
			}
		}
	}
}
/*static void eventCallback(void *data, int id, enum ui_event ev)
{
	struct mydata *mydat = data;
	if(ev == UI_EVENT_KEYDOWN){
		printf("Key down: %d\n", id);
		return;
	}
	if(ev == UI_EVENT_KEYUP){
		printf("Key up: %d\n", id);
		return;
	}
	mydat->win->widgets[0]->data = (void*) "Hello, Clicker!";
	redrawWidget(mydat->uiinf, mydat->win->widgets[0]);
}*/

int main(){
	struct mydata mydat;
	struct sockaddr_un addr = {AF_UNIX, "/tmp/bananui.sock"};
	signal(SIGPIPE, SIG_IGN);
	/*struct ui_cbx_info cbxdata;
	struct ui_widget testcbx;
	struct ui_widget testlabel1;
	struct ui_widget testlabel2;
	struct ui_widget testlabel3;
	struct ui_widget testbutton1;
	struct ui_widget testbutton2;
	struct ui_widget testbutton3;*/
	mydat.uiinf = getui();
	/*mydat.win = createWindow(mydat.uiinf);
	mydat.win->event = eventCallback;
	mydat.win->userdata = &mydat;
	strcpy(mydat.win->title, "Home Window");*/
	mydat.nsockfds = 1;
	mydat.sockfds[0] = socket(AF_UNIX, SOCK_STREAM, 0);
	if(mydat.sockfds[0] < 0){
		perror("socket");
		exit(5);
	}
	fcntl(mydat.sockfds[0], F_SETFL, O_CLOEXEC);
	if(bind(mydat.sockfds[0], (struct sockaddr*) &addr, sizeof(addr)) <
		0)
	{
		perror("Failed to bind to /tmp/bananui.sock");
	}
	if(listen(mydat.sockfds[0], MAX_WAITING_CLIENTS) < 0){
		perror("listen");
		exit(5);
	}
	chmod("/tmp/bananui.sock", 0666);
	/*testlabel1.type = UI_LABEL;
	testlabel1.textalign = UI_TA_LEFT;
	testlabel1.rightend = mydat.uiinf->fbinf->vinfo->xres;
	testlabel1.data = (void*) "Hello, World!";
	testlabel1.freedata = 0;
	testlabel2.type = UI_LABEL;
	testlabel2.textalign = UI_TA_CENTER;
	testlabel2.rightend = mydat.uiinf->fbinf->vinfo->xres;
	testlabel2.data = (void*) "Hello, World!";
	testlabel2.freedata = 0;
	testlabel3.type = UI_LABEL;
	testlabel3.textalign = UI_TA_RIGHT;
	testlabel3.rightend = mydat.uiinf->fbinf->vinfo->xres;
	testlabel3.data = (void*) "Hello, World!";
	testlabel3.freedata = 0;
	addWidget(mydat.uiinf, mydat.win, &testlabel1, 0);
	addWidget(mydat.uiinf, mydat.win, &testlabel2, 0);
	addWidget(mydat.uiinf, mydat.win, &testlabel3, 0);
	redrawWindow(mydat.uiinf, mydat.win);
	sleep(1);
	cbxdata.first = cbxdata.last = cbxdata.firstfoc = cbxdata.lastfoc =
		NULL;
	cbxdata.numwidgets = 0;
	testcbx.type = UI_CBX;
	testcbx.rightend = mydat.uiinf->fbinf->vinfo->xres;
	testcbx.data = &cbxdata;
	testcbx.freedata = 0;
	testbutton1.type = UI_BUTTON;
	testbutton1.rightend = mydat.uiinf->fbinf->vinfo->xres;
	testbutton1.data = (void*) "Button";
	testbutton1.freedata = 0;
	addWidgetToCbx(mydat.uiinf, mydat.win, &testcbx, &testbutton1, 1);
	testbutton2.type = UI_BUTTON;
	testbutton2.rightend = mydat.uiinf->fbinf->vinfo->xres;
	testbutton2.data = (void*) "Button2";
	testbutton2.freedata = 0;
	addWidgetToCbx(mydat.uiinf, mydat.win, &testcbx, &testbutton2, 1);
	testbutton3.type = UI_BUTTON;
	testbutton3.rightend = mydat.uiinf->fbinf->vinfo->xres;
	testbutton3.data = (void*) "Button3";
	testbutton3.freedata = 0;
	addWidgetToCbx(mydat.uiinf, mydat.win, &testcbx, &testbutton3, 1);
	addWidget(mydat.uiinf, mydat.win, &testcbx, 1);
	redrawWindow(mydat.uiinf, mydat.win);*/
	if(fork() == 0){
		execl("/usr/bin/bananui-login", "bananui-login", NULL);
		perror("/usr/bin/bananui-login");
		exit(2);
	}
	uiLoop(mydat.uiinf, mydat.sockfds, &mydat.nsockfds,
		fdCallback, &mydat);
	uicleanup(mydat.uiinf);
	return 0;
}
