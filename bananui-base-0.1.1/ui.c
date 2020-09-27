/*
	The BananUI user interface. For the socket and command implementation,
	see uiserv.c.
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
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <linux/fb.h>
#include <linux/input.h>
#include "font_10x18.h"
#include "gr.h"
#include "ui.h"
#define BG_R 0xA0
#define BG_G 0xA0
#define BG_B 0x80
#define BOX_H 60

unsigned char *decompr_fnt;

static void decompress_font(){
	unsigned char compr_byte, *in, *out;
	in = fnt.rundata;
	decompr_fnt = malloc(fnt.height * fnt.width);
	out = decompr_fnt;
	while((compr_byte = *in++)){
		memset(out, compr_byte & 0x80, compr_byte & 0x7f);
		out += compr_byte & 0x7f;
	}
}

static int text(struct uiinfo *uiinf, int x, int y, const char *txt,
	int r, int g, int b, int a)
{
	char ch;
	int height;
	height = fnt.cheight;
	while((ch = *txt++)){
		int i, j;
		int offset;
		offset = (ch - ' ') * (fnt.cwidth);
		for(i = 0; i < fnt.cheight; i++){
			for(j = 0; j < fnt.cwidth; j++){
				if(decompr_fnt[(j + offset)])
				{
					setPixel(uiinf->fbinf,
						y+i, j+x, r, g, b, a);
				}
			}
			offset += fnt.width;
		}
		x += fnt.cwidth;
		if(x >= uiinf->fbinf->vinfo->xres){
			x = 0;
			y += fnt.cheight;
			height += fnt.cheight;
		}
	}
	return height;
}

static void box(struct uiinfo *uiinf, int x, int y, int w, int br, int bg,
	int bb, int bgr, int bgg, int bgb)
{
	drawRect(uiinf->fbinf, y, x, BOX_H, w, br, bg, bb, 255);
	drawRect(uiinf->fbinf, y+5, x+5, BOX_H-10, w-10, bgr, bgg, bgb,
		255);
}

static void displayLabel(struct uiinfo *uiinf, struct ui_widget *widget){
	int offset;
	char *txt;
	txt = (char*) widget->data;
	if(widget->type != UI_LABEL) return;
	switch(widget->textalign){
		case UI_TA_LEFT:
			offset = 0;
			break;
		case UI_TA_CENTER:
			offset = widget->rightend / 2 -
				strlen(txt) * fnt.cwidth / 2;
			break;
		case UI_TA_RIGHT:
			offset = widget->rightend -
				strlen(txt) * fnt.cwidth;
			break;
	}
	widget->height = text(uiinf, widget->x + offset, widget->y, txt,
		get_red(widget->style.fgcolor),
		get_green(widget->style.fgcolor),
		get_blue(widget->style.fgcolor),
		get_alpha(widget->style.fgcolor));
}
static void displayImage(struct uiinfo *uiinf, struct ui_widget *widget){
	int offset, width, height, i, j;
	unsigned int *data;
	unsigned char *chardata;
	chardata = widget->data;
	data = (unsigned int*) (chardata+2);
	width = chardata[0];
	height = chardata[1];
	if(widget->type != UI_IMAGE) return;
	switch(widget->textalign){
		case UI_TA_LEFT:
			offset = 0;
			break;
		case UI_TA_CENTER:
			offset = widget->rightend / 2 - width / 2;
			break;
		case UI_TA_RIGHT:
			offset = widget->rightend - width;
			break;
	}
	for(i = 0; i < height; i++){
		for(j = 0; j < width; j++){
			setPixel(uiinf->fbinf, widget->y+i,
				offset+widget->x+j, data[i*width+j] &0xff,
				(data[i*width+j] >> 8) & 0xff,
				(data[i*width+j] >> 16) & 0xff,
				(data[i*width+j] >> 24) & 0xff);
		}
	}
	widget->height = height;
}
static void displayButton(struct uiinfo *uiinf, struct ui_widget *widget){
	char *txt;
	int border_blue = 0x40;
	if(widget->type != UI_BUTTON && widget->type != UI_FOBUTTON)
		return;
	if(widget->type == UI_FOBUTTON) border_blue = 0xC0;
	txt = (char*) widget->data;
	box(uiinf, widget->x, widget->y, widget->rightend - widget->x,
		0x60, 0x60, border_blue, BG_R, BG_G, BG_B);
	text(uiinf, widget->x + 10, widget->y + 20, txt,
		get_red(widget->style.fgcolor),
		get_green(widget->style.fgcolor),
		get_blue(widget->style.fgcolor),
		get_alpha(widget->style.fgcolor));
	widget->height = BOX_H;
}

static void displayInput(struct uiinfo *uiinf, struct ui_widget *widget)
{
	char *txt, tmp;
	int border_blue = 0x40, scrolloffset, cursorpos;
	cursorpos = ((unsigned char *)widget->data)[1];
	scrolloffset = ((unsigned char *)widget->data)[0];
	if(widget->type != UI_INPUT && widget->type != UI_FOINPUT)
		return;
	if(widget->type == UI_FOINPUT){
		border_blue = 0xC0;
	}
	txt = ((char*)widget->data)+2;
	box(uiinf, widget->x, widget->y, widget->rightend - widget->x,
		0x60, 0x60, border_blue, 255, 255, 255);
	tmp = txt[scrolloffset + ((widget->rightend - widget->x) /
			fnt.cwidth) - 2];
	txt[scrolloffset + ((widget->rightend - widget->x) /
			fnt.cwidth) - 2] = 0;
	text(uiinf, widget->x + 10, widget->y + 20, txt+scrolloffset,
		0, 0, 0, 255);
	txt[scrolloffset + ((widget->rightend - widget->x) / fnt.cwidth)
		- 2] = tmp;
	if(widget->type == UI_FOINPUT){
		drawRect(uiinf->fbinf, widget->y+17, widget->x + 10 +
			(fnt.cwidth * (cursorpos-scrolloffset)),
			BOX_H-40, 1, 0, 0, 0, 255);
	}
	widget->height = BOX_H;
}
int getApproximateWidth(struct uiinfo *uiinf, struct ui_widget *widget,
	int n)
{
	switch(widget->type){
		case UI_BUTTON:
		case UI_FOBUTTON:
		case UI_INPUT:
		case UI_FOINPUT:
			return uiinf->fbinf->vinfo->xres / n;
		case UI_LABEL:
			return strlen((char*)widget->data) * fnt.cwidth;
		case UI_IMAGE:
			return ((unsigned char*)widget->data)[0];
		case UI_CBX:
			fprintf(stderr, "Nested CBX is not possible!\n");
	}
	return 0;
}
void addWidgetToCbx(struct uiinfo *uiinf, struct ui_window *win,
	struct ui_widget *cbx, struct ui_widget *widget, int focusable)
{
	struct ui_cbx_info *cbxi = cbx->data;
	widget->cbx = cbx;
	if(cbxi->last){
		cbxi->last->right = widget;
	}
	else {
		cbxi->first = widget;
	}
	widget->right = NULL;
	widget->left = cbxi->last;
	if(focusable){
		widget->focusable = 1;
		if(cbxi->lastfoc){
			cbxi->lastfoc->rightfoc = widget;
		}
		else {
			cbxi->firstfoc = widget;
		}
		widget->leftfoc = cbxi->lastfoc;
		widget->rightfoc = NULL;
		cbxi->lastfoc = widget;
	}
	win->widgets[win->nextwidgetid] = widget;
	widget->id = win->nextwidgetid;
	win->nextwidgetid++;
	cbxi->last = widget;
	cbxi->numwidgets++;
}
static void displayCbx(struct uiinfo *uiinf, struct ui_window *win,
	struct ui_widget *cbx)
{
	struct ui_widget *widget;
	struct ui_cbx_info *cbxi = cbx->data;
	int x = 0, height = 0;
	for(widget = cbxi->first; widget;
			widget = widget->right)
	{
		widget->y = cbx->y;
		widget->x = x;
		widget->rightend = x += getApproximateWidth(uiinf, widget,
			cbxi->numwidgets);
		redrawWidget(uiinf, win, widget);
		if(widget->height > height) height = widget->height;
		widget->nextfoc = cbx->nextfoc;
		widget->prevfoc = cbx->prevfoc;
	}
	cbx->height = height;
}

static void displayWidget(struct uiinfo *uiinf, struct ui_window *win,
	struct ui_widget *widget)
{
	switch(widget->type){
		case UI_BUTTON:
		case UI_FOBUTTON:
			displayButton(uiinf, widget);
			break;
		case UI_IMAGE:
			displayImage(uiinf, widget);
			break;
		case UI_LABEL:
			displayLabel(uiinf, widget);
			break;
		case UI_INPUT:
		case UI_FOINPUT:
			displayInput(uiinf, widget);
			break;
		case UI_CBX:
			displayCbx(uiinf, win, widget);
			break;
		default:
			fprintf(stderr, "Unknown widget type %d\n",
				widget->type);
	}
}

void redrawWidget(struct uiinfo *uiinf, struct ui_window *win,
	struct ui_widget *widget)
{
	widget->y += win->actual_yoffset;
	displayWidget(uiinf, win, widget);
	drawRect(uiinf->fbinf, widget->y,
		widget->x, widget->height, widget->rightend - widget->x,
		get_red(widget->style.bgcolor),
		get_green(widget->style.bgcolor),
		get_blue(widget->style.bgcolor), 255);
	displayWidget(uiinf, win, widget);
	widget->y -= win->actual_yoffset;
}

static void redrawWindowBuffer(struct uiinfo *uiinf, unsigned char *buffer,
	struct ui_window *win);
void focus(struct uiinfo *uiinf, struct ui_window *win,
	struct ui_widget *widget)
{
	if(win->focused){
		switch(win->focused->type){
			case UI_FOBUTTON:
				win->focused->type = UI_BUTTON;
				break;
			case UI_FOINPUT:
				win->focused->type = UI_INPUT;
				break;
			case UI_CBX:
				break;
			default:
				fprintf(stderr, "win->focused is not focused\n");
		}
	}
	if(!widget){
		win->focused = NULL;
		return;
	}
	if(widget->y < 0){
		win->scrolloffset += widget->y;
	}
	else if(widget->y + widget->height >
		win->height)
	{
		win->scrolloffset += (widget->y + widget->height) -
			win->height;
	}
	switch(widget->type){
		case UI_BUTTON:
			widget->type = UI_FOBUTTON;
			break;
		case UI_INPUT:
			widget->type = UI_FOINPUT;
			break;
		case UI_CBX: {
			struct ui_widget *subwidget;
			int i;
			subwidget = ((struct ui_cbx_info*)widget->data)->
				firstfoc;
			if(!subwidget){
				focus(uiinf, win, widget->nextfoc);
				return;
			}
			for(i = 0; subwidget->right &&
				i < win->cbx_focus_index; i++)
			{
				subwidget = subwidget->right;
			}
			focus(uiinf, win, subwidget);
			return;
		}
		default:
			fprintf(stderr, "Widget already focused or not focusable\n");
	}
	win->focused = widget;
	redrawWindow(uiinf, win);
}

void destroyWidget(struct uiinfo *uiinf, struct ui_widget *widget)
{
	if(widget->freedata) free(widget->data);
	free(widget);
}

void destroyWindow(struct uiinfo *uiinf, struct ui_window *win)
{
	int i;
	unsigned char *tmpbuf1, *tmpbuf2;
	uiinf->windows[win->id] = NULL;
	uiinf->curwindow = win->parent ? win->parent : uiinf->windows[0];
	tmpbuf1 = malloc(uiinf->fbinf->finfo->smem_len);
	tmpbuf2 = malloc(uiinf->fbinf->finfo->smem_len);
	if(uiinf->curwindow)
		redrawWindowBuffer(uiinf, tmpbuf1, uiinf->curwindow);
	else
		memset(tmpbuf1, 0, uiinf->fbinf->finfo->smem_len);
	redrawWindowBuffer(uiinf, tmpbuf2, win);
	for(i = 0; i <= 7; i++){
		fbcpy(uiinf->fbinf, tmpbuf1, 0);
		fbcpy(uiinf->fbinf, tmpbuf2,
			win->actual_yoffset + (i * 40));
		refreshScreen(uiinf->fbinf);
	}
	free(tmpbuf1);
	free(tmpbuf2);
	for(i = 0; i < MAX_WIDGETS; i++){
		if(win->widgets[i]) destroyWidget(uiinf, win->widgets[i]);
	}
	while(uiinf->windows[uiinf->nextwindowid-1] == NULL)
		uiinf->nextwindowid--;
	redrawWindow(uiinf, uiinf->curwindow);
	free(win);
}

void clearWindow(struct uiinfo *uiinf, struct ui_window *win)
{
	int i;
	win->focused = NULL;
	for(i = 0; i < MAX_WIDGETS; i++){
		if(win->widgets[i]) destroyWidget(uiinf, win->widgets[i]);
		win->widgets[i] = NULL;
	}
	win->firstfocusable = NULL;
	win->lastfocusable = NULL;
	win->firstwidget = NULL;
	win->lastwidget = NULL;
	win->nextwidgetid = 0;
	win->scrolloffset = 0;
}

static void initWindowStyle(struct ui_window *win){
	win->skbgr = win->skbgg = win->skbgb = 0x80;
	win->skbga = 255;
	win->skfgr = win->skfgg = win->skfgb = 0;
	win->bgr = win->bgg = win->bgb = 0xA0;
}

struct ui_window *createWindow(struct uiinfo *uiinf)
{
	struct ui_window *win;
	unsigned char *tmpbuf;
	int i;
	if(uiinf->nextwindowid == MAX_WINDOWS){
		fprintf(stderr, "Too many windows!\n");
		exit(4);
	}
	win = malloc(sizeof(struct ui_window));
	initWindowStyle(win);
	uiinf->windows[uiinf->nextwindowid] = win;
	uiinf->curwindow = win;
	win->id = uiinf->nextwindowid;
	uiinf->nextwindowid++;
	win->cbx_focus_index = 0;
	for(i = 0; i < MAX_WIDGETS; i++){
		win->widgets[i] = NULL;
	}
	win->firstwidget = NULL;
	win->lastwidget = NULL;
	win->firstfocusable = NULL;
	win->lastfocusable = NULL;
	win->focused = NULL;
	win->parent = NULL;
	win->scrolloffset = 0;
	win->maxscroll = -1; /* tell redrawWindow to compute it */
	win->nextwidgetid = 0;
	win->hidetoppanel = 0;
	win->hideskpanel = 0;
	win->yoffset = 0;
	win->current_skl[0] = win->current_skc[0] = win->current_skr[0] = 0;
	tmpbuf = malloc(uiinf->fbinf->finfo->smem_len);
	redrawWindowBuffer(uiinf, tmpbuf, win);
	for(i = 0; i <= 7; i++){
		fbcpy(uiinf->fbinf, tmpbuf,
			uiinf->fbinf->vinfo->yres - (i * 40));
		refreshScreen(uiinf->fbinf);
	}
	free(tmpbuf);
	redrawWindow(uiinf, win);
	return win;
}
void showTopPanel(struct uiinfo *uiinf)
{
	struct tm *curtime;
	char timestr[6];
	time_t t;
	t = time(NULL);
	curtime = localtime(&t);
	if(!curtime){
		perror("localtime");
	}
	drawRect(uiinf->fbinf, 0, 0, 25, uiinf->fbinf->vinfo->xres,
		0, 0, 0, 255);
	if(strftime(timestr, 6, "%R", curtime))
		text(uiinf, 0, 0, timestr, 255, 255, 255, 255);
}
void showSoftkeys(struct uiinfo *uiinf, struct ui_window *win)
{
	int loffset, coffset, roffset, text_y;
	drawRect(uiinf->fbinf, uiinf->fbinf->vinfo->yres - 25, 0, 25,
		uiinf->fbinf->vinfo->xres, win->skbgr, win->skbgg,
		win->skbgb, win->skbga);
	text_y = uiinf->fbinf->vinfo->yres - 20;
	loffset = 5;
	roffset = uiinf->fbinf->vinfo->xres - 5 - strlen(win->current_skr);
	coffset = uiinf->fbinf->vinfo->xres / 2 -
		(strlen(win->current_skc) * fnt.cwidth) / 2;
	text(uiinf, loffset, text_y, win->current_skl, win->skfgr,
		win->skfgg, win->skfgb, 255);
	text(uiinf, coffset, text_y, win->current_skc, win->skfgr,
		win->skfgg, win->skfgb, 255);
	text(uiinf, roffset, text_y, win->current_skr, win->skfgr,
		win->skfgg, win->skfgb, 255);
}
static void redrawWindowBuffer(struct uiinfo *uiinf, unsigned char *buffer,
	struct ui_window *win)
{
	struct ui_widget *tmp;
	unsigned char *oldbuffer;
	if(!win) return;
	oldbuffer = uiinf->fbinf->framebuffer;
	uiinf->fbinf->framebuffer = buffer;
	win->height = uiinf->fbinf->vinfo->yres;
	drawRect(uiinf->fbinf, win->yoffset, 0, win->height,
		uiinf->fbinf->vinfo->xres, win->bgr, win->bgg, win->bgb,
		255);
	if(win->firstwidget){
		win->firstwidget->y = -(win->scrolloffset) +
			win->yoffset;
	}
	for(tmp = win->firstwidget; tmp; tmp = tmp->next){
		redrawWidget(uiinf, win, tmp);
		if(tmp->next){
			tmp->next->y = tmp->y + tmp->height;
		}
		win->maxscroll = tmp->y + tmp->height +
			win->scrolloffset - win->height;
	}
	uiinf->fbinf->framebuffer = oldbuffer;
}
void redrawWindow(struct uiinfo *uiinf,
	struct ui_window *win)
{
	unsigned char *tmpbuf;
	if(!win){
		drawRect(uiinf->fbinf, 0, 0, uiinf->fbinf->vinfo->yres,
			uiinf->fbinf->vinfo->xres, 0, 0, 0, 255);
		goto end;
	}
	tmpbuf = malloc(uiinf->fbinf->finfo->smem_len);
	win->actual_yoffset = win->yoffset;
	redrawWindowBuffer(uiinf, tmpbuf, win);
	fbcpy(uiinf->fbinf, tmpbuf,
		win->hidetoppanel ? 0 : 25);
	free(tmpbuf);
	if(!win->hideskpanel){
		win->height -= 25;
		showSoftkeys(uiinf, win);
	}
	if(!win->hidetoppanel){
		win->actual_yoffset += 25;
		win->height -= 25;
		showTopPanel(uiinf);
	}
end:
	refreshScreen(uiinf->fbinf);
}

void addWidget(struct uiinfo *uiinf, struct ui_window *win,
	struct ui_widget *widget, int focusable)
{
	if(win->nextwidgetid == MAX_WIDGETS){
		fprintf(stderr, "Too many widgets\n");
		return;
	}
	win->widgets[win->nextwidgetid] = widget;
	widget->id = win->nextwidgetid;
	win->nextwidgetid++;
	widget->focusable = 0;
	widget->x = 0;
	widget->next = NULL;
	widget->nextfoc = NULL;
	widget->right = NULL;
	widget->rightfoc = NULL;
	widget->leftfoc = NULL;
	widget->cbx = NULL;
	widget->prev = win->lastwidget;
	if(!win->firstwidget){
		win->firstwidget = widget;
	}
	else {
		win->lastwidget->next = widget;
	}
	win->lastwidget = widget;
	if(focusable){
		widget->focusable = 1;
		if(!win->firstfocusable){
			win->firstfocusable = widget;
		}
		else {
			win->lastfocusable->nextfoc = widget;
		}
		widget->prevfoc = win->lastfocusable;
		win->lastfocusable = widget;
		if(!win->focused){
			redrawWindow(uiinf, win);
			focus(uiinf, win, widget);
		}
	}
}

static int handleInputLeft(struct uiinfo *uiinf, struct ui_widget *inp)
{
	unsigned char *cursorpos, *scrolloffset;
	cursorpos = ((unsigned char*)inp->data)+1;
	scrolloffset = ((unsigned char*)inp->data);
	if(*cursorpos == 0) return 0;
	if(*scrolloffset == *cursorpos){
		(*scrolloffset)--;
	}
	(*cursorpos)--;
	redrawWidget(uiinf, uiinf->curwindow, inp);
	refreshScreen(uiinf->fbinf);
	return 1;
}
static int handleInputRight(struct uiinfo *uiinf, struct ui_widget *inp)
{
	unsigned char *cursorpos, *scrolloffset;
	scrolloffset = (unsigned char*)inp->data;
	cursorpos = ((unsigned char*)inp->data)+1;
	if(*cursorpos == strlen(((char*)inp->data)+2)) return 0;
	if(*cursorpos == *scrolloffset + ((inp->rightend - inp->x) /
		fnt.cwidth) - 2)
	{
		(*scrolloffset)++;
	}
	(*cursorpos)++;
	redrawWidget(uiinf, uiinf->curwindow, inp);
	refreshScreen(uiinf->fbinf);
	return 1;
}

static char handleNumeric(struct uiinfo *uiinf, int n)
{
	switch(uiinf->inptype){
		case INPTYPE_NUMERIC:
			if(n < 10) return n + '0';
			if(n == 11){ /* # */
				uiinf->inptype = INPTYPE_ALPHA;
			}
		case INPTYPE_ALPHA:
			if(n != uiinf->curkey){
				uiinf->curkey = n;
				uiinf->keyindex = 0;
			}
			else {
				uiinf->keyindex++;
			}
			return keymap[uiinf->curkey][uiinf->keyindex];
			break;
}

static int keyIn(struct uiinfo *uiinf, struct ui_window *win, int n)
{
	char ch, *text;
	int i;
	unsigned char *cursorpos, *scrolloffset;
	if(!win->focused || win->focused->type != UI_FOINPUT) return 0;
	scrolloffset = ((unsigned char*)win->focused->data);
	cursorpos = ((unsigned char*)win->focused->data)+1;
	text = ((char*)win->focused->data)+2;
	for(i = 254; i >= *cursorpos; i--){
		text[i+1] = text[i];
	}
	text[255] = 0;
	text[*cursorpos] = ch = handleNumeric(uiinf, n);
	if(*cursorpos == *scrolloffset + ((win->focused->rightend -
		win->focused->x) / fnt.cwidth) - 2)
	{
		(*scrolloffset)++;
	}
	(*cursorpos)++;
	redrawWidget(uiinf, win, win->focused);
	refreshScreen(uiinf->fbinf);
	return 1;
}
static void handleKeyup(struct uiinfo *uiinf, struct ui_window *win,
	int code)
{
	win->event(win->userdata, code, UI_EVENT_KEYUP);
}
static void handleSlideClose(struct uiinfo *uiinf)
{
#ifndef DESKTOP
	int ledfd;
	uiinf->on = 0;
	ledfd = open("/sys/class/leds/button-backlight/brightness",
		O_WRONLY);
	if(ledfd < 0){
		perror("/sys/class/leds/button-backlight/brightness");
		return;
	}
	write(ledfd, "0\n", 2);
	close(ledfd);
	ledfd = open("/sys/class/leds/lcd-backlight/brightness",
		O_WRONLY);
	if(ledfd < 0){
		perror("/sys/class/leds/lcd-backlight/brightness");
		return;
	}
	write(ledfd, "0\n", 2);
	close(ledfd);
#endif
}
static void handleSlideOpen(struct uiinfo *uiinf)
{
#ifndef DESKTOP
	int ledfd;
	uiinf->on = 1;
	ledfd = open("/sys/class/leds/button-backlight/brightness",
		O_WRONLY);
	if(ledfd < 0){
		perror("/sys/class/leds/button-backlight/brightness");
		return;
	}
	write(ledfd, "40\n", 4);
	close(ledfd);
	ledfd = open("/sys/class/leds/lcd-backlight/brightness",
		O_WRONLY);
	if(ledfd < 0){
		perror("/sys/class/leds/lcd-backlight/brightness");
		return;
	}
	write(ledfd, "255\n", 4);
	close(ledfd);
	refreshScreen(uiinf->fbinf);
#endif
}
static void handleSlideFullopen(struct uiinfo *uiinf)
{
}
struct window_list_data {
	struct ui_window *win, *prevcur;
	struct uiinfo *uiinf;
	int windowids[MAX_WINDOWS];
};
static void closeWindowList(struct window_list_data *listdata,
	struct ui_window *open)
{
	destroyWindow(listdata->uiinf, listdata->win);
	listdata->uiinf->curwindow = open;
	listdata->uiinf->listingwindows = 0;
	redrawWindow(listdata->uiinf, open);
	free(listdata);
}
static void windowListEventCallback(void *data, int id, enum ui_event evt)
{
	struct window_list_data *listdata = data;
	if(evt == UI_EVENT_CLICK){
		closeWindowList(listdata, listdata->uiinf->windows[
			listdata->windowids[id]]);
	}
	else if(evt == UI_EVENT_EXIT){
		closeWindowList(listdata, listdata->prevcur);
	}
}
static void showWindowList(struct uiinfo *uiinf)
{
	struct ui_widget *btn;
	struct window_list_data *listdata;
	int i;
	if(uiinf->listingwindows) return;
	uiinf->listingwindows = 1;
	listdata = malloc(sizeof(struct window_list_data));
	listdata->prevcur = uiinf->curwindow;
	listdata->win = createWindow(uiinf);
	listdata->uiinf = uiinf;
	strcpy(listdata->win->title, "Window list");
	listdata->win->bgr = 0xE0;
	listdata->win->bgg = 0xE0;
	listdata->win->bgb = 0xE0;
	listdata->win->skbgr = 0xC0;
	listdata->win->skbgg = 0xC0;
	listdata->win->skbgb = 0xC0;
	for(i = 0; i < uiinf->nextwindowid; i++){
		if(uiinf->windows[i] && uiinf->windows[i] != listdata->win){
			btn = malloc(sizeof(struct ui_widget));
			btn->type = UI_BUTTON;
			btn->rightend = uiinf->fbinf->vinfo->xres;
			btn->data = uiinf->windows[i]->title;
			btn->freedata = 0;
			btn->style.fgcolor = from_rgb(0, 0, 0);
			btn->style.bgcolor = from_rgb(0xC0, 0xC0, 0xC0);
			addWidget(uiinf, listdata->win, btn, 1);
			listdata->windowids[btn->id] = i;
		}
	}
	listdata->win->event = windowListEventCallback;
	listdata->win->userdata = listdata;
	redrawWindow(uiinf, listdata->win);
}
static void handlePowerKey(struct uiinfo *uiinf)
{
	if(!uiinf->on) handleSlideOpen(uiinf);
	else switch(uiinf->slidestate){
		case SLIDE_CLOSED:
			handleSlideClose(uiinf);
			break;
		case SLIDE_FULLOPEN:
		case SLIDE_HALFOPEN:
			showWindowList(uiinf);
			break;
		default:
			break;
	}
}
static void handleKeydown(struct uiinfo *uiinf, struct ui_window *win,
	int code)
{
	if(!win) return;
	switch(code){
		case KEY_DOWN:
			if(win->focused && win->focused->nextfoc &&
				win->focused->nextfoc->y <= (win->height +
				win->height/2))
			{
				focus(uiinf, win, win->focused->nextfoc);
			}
			else if(win->scrolloffset < win->maxscroll){
				win->scrolloffset += 25;
				if(win->scrolloffset > win->maxscroll)
					win->scrolloffset = win->maxscroll;
				redrawWindow(uiinf, win);
			}
			break;
		case KEY_UP:
			if(win->focused && win->focused->prevfoc &&
				win->focused->prevfoc->y >= -win->height/2)
			{
				focus(uiinf, win, win->focused->prevfoc);
			}
			else if(win->scrolloffset > 0){
				win->scrolloffset -= 25;
				if(win->scrolloffset < 0)
					win->scrolloffset = 0;
				redrawWindow(uiinf, win);
			}
			break;
		case KEY_OK:
			if(win->focused)
				win->event(win->userdata, win->focused->id,
					UI_EVENT_CLICK);
			break;
		case KEY_LEFT:
			if(win->focused && win->focused->type == UI_FOINPUT
				&& handleInputLeft(uiinf, win->focused))
			{
				break;
			}
			if(win->focused && win->focused->leftfoc){
				win->cbx_focus_index--;
				focus(uiinf, win, win->focused->leftfoc);
			}
			break;
		case KEY_RIGHT:
			if(win->focused && win->focused->type == UI_FOINPUT
				&& handleInputRight(uiinf, win->focused))
			{
				break;
			}
			if(win->focused && win->focused->rightfoc){
				win->cbx_focus_index++;
				focus(uiinf, win, win->focused->rightfoc);
			}
			break;
		case KEY_1: keyIn(uiinf, win, 1); break;
		case KEY_2: keyIn(uiinf, win, 2); break;
		case KEY_3: keyIn(uiinf, win, 3); break;
		case KEY_4: keyIn(uiinf, win, 4); break;
		case KEY_5: keyIn(uiinf, win, 5); break;
		case KEY_6: keyIn(uiinf, win, 6); break;
		case KEY_7: keyIn(uiinf, win, 7); break;
		case KEY_8: keyIn(uiinf, win, 8); break;
		case KEY_9: keyIn(uiinf, win, 9); break;
		case KEY_0: keyIn(uiinf, win, 0); break;
		case KEY_NUMERIC_STAR: keyIn(uiinf, win, 10); break;
		case KEY_NUMERIC_POUND: keyIn(uiinf, win, 11); break;
		case KEY_BACKSPACE: {
			char *text;
			int i;
			unsigned char *cursorpos, *scrolloffset;
			if(!win->focused || win->focused->type !=
				UI_FOINPUT)
			{
				win->event(win->userdata, 0, UI_EVENT_EXIT);
				break;
			}
			scrolloffset = ((unsigned char*)win->focused->data);
			cursorpos = ((unsigned char*)win->focused->data)+1;
			if(*cursorpos == 0){
				win->event(win->userdata, 0, UI_EVENT_EXIT);
				break;
			}
			text = ((char*)win->focused->data)+2;
			for(i = *cursorpos; i < 255; i++){
				text[i-1] = text[i];
			}
			text[255] = 0;
			if(*scrolloffset > 0) (*scrolloffset)--;
			(*cursorpos)--;
			redrawWidget(uiinf, win, win->focused);
			refreshScreen(uiinf->fbinf);
			break;
		}
		case KEY_POWER:
			handlePowerKey(uiinf);
			break;
	}
	win->event(win->userdata, code, UI_EVENT_KEYDOWN);
}
void uiLoop(struct uiinfo *uiinf, int *custom_fds, int *ncustom_fds,
	fd_callback custom_callback, void *userdata)
{
	int i, mfds = 0;
	struct timeval timeout;
	fd_set readfds;
	while(1){
		int res;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		FD_ZERO(&readfds);
		for(i = 0; i < uiinf->nfds; i++){
			FD_SET(uiinf->inpfds[i], &readfds);
			if(uiinf->inpfds[i] >= mfds)
				mfds = uiinf->inpfds[i]+1;
		}
		for(i = 0; i < *ncustom_fds; i++){
			FD_SET(custom_fds[i], &readfds);
			if(custom_fds[i] >= mfds)
				mfds = custom_fds[i]+1;
		}
		res = select(mfds, &readfds, NULL, NULL, uiinf->on ? &timeout : NULL);
		if(res < 0){
			perror("select");
			exit(1);
		}
		if(res == 0){
			if(uiinf->curwindow && !uiinf->curwindow->hidetoppanel){
				showTopPanel(uiinf);
				refreshScreen(uiinf->fbinf);
			}
			continue;
		}
		for(i = 0; i < uiinf->nfds; i++){
			if(FD_ISSET(uiinf->inpfds[i], &readfds)){
				struct input_event event;
				read(uiinf->inpfds[i], &event,
					sizeof(struct input_event));
				if(event.type == EV_KEY &&
					event.value == 1) /* Key Down */
				{
					if(event.code == KEY_POWER &&
						i == 4)
					{
						event.code = KEY_BACKSPACE;
					}
					handleKeydown(uiinf,
						uiinf->curwindow,
						event.code);
				}
				else if(event.type == EV_KEY &&
					event.value == 0) /* Key Up */
				{
					handleKeyup(uiinf,
						uiinf->curwindow,
						event.code);
				}
				else if(event.type == EV_SW &&
					(i == 1 || i == 2) && event.value == 0)
				{
					uiinf->slidestate = SLIDE_HALFOPEN;
					handleSlideOpen(uiinf);
				}
				else if(event.type == EV_SW && i == 1 &&
					event.value == 1)
				{
					uiinf->slidestate = SLIDE_CLOSED;
					handleSlideClose(uiinf);
				}
				else if(event.type == EV_SW && i == 2 &&
					event.value == 1)
				{
					uiinf->slidestate = SLIDE_FULLOPEN;
					handleSlideFullopen(uiinf);
				}
			}
		}
		for(i = 0; i < *ncustom_fds; i++){
			if(FD_ISSET(custom_fds[i], &readfds)){
				custom_callback(userdata, i);
			}
		}
	}
}
void makeUnfocusable(struct uiinfo *uiinf, struct ui_window *win,
	struct ui_widget *widget)
{
	widget->focusable = 0;
	if(win->focused == widget) win->focused =
		win->focused->nextfoc ? win->focused->nextfoc :
			win->focused->prevfoc;
	if(!widget->cbx){
		if(widget->prevfoc){
			widget->prevfoc->nextfoc = widget->nextfoc;
		}
		else {
			win->firstfocusable = widget->nextfoc;
		}
		if(widget->nextfoc){
			widget->nextfoc->prevfoc = widget->prevfoc;
		}
		else {
			win->lastfocusable = widget->prevfoc;
		}
	}
	else {
		if(widget->leftfoc){
			widget->leftfoc->rightfoc = widget->rightfoc;
		}
		else {
			((struct ui_cbx_info*)widget->cbx->data)->firstfoc=
				widget->rightfoc;
		}
		if(widget->cbx && widget->rightfoc){
			widget->rightfoc->leftfoc = widget->leftfoc;
		}
		else {
			((struct ui_cbx_info*)widget->cbx->data)->lastfoc =
				widget->leftfoc;
		}
	}
}

void makeFocusable(struct uiinfo *uiinf, struct ui_window *win,
	struct ui_widget *widget)
{
	if(!widget->cbx){
		if(!win->firstfocusable){
			redrawWindow(uiinf, win);
			focus(uiinf, win, widget);
		}
		/*	Find the next focusable widget -
			it has the focusable flag set. */
		for(widget->nextfoc = widget;
			widget->nextfoc && !widget->nextfoc->focusable;
			widget->nextfoc = widget->nextfoc->next) {}
		if(widget->nextfoc){
			widget->prevfoc = widget->nextfoc->prevfoc;
			widget->nextfoc->prevfoc = widget;
		}
		else {
			win->lastfocusable = widget;
			/*	Find the previous focusable widget -
				no nextfoc has been found. */
			for(widget->prevfoc = widget;
				widget->prevfoc &&
				!widget->prevfoc->focusable;
				widget->prevfoc = widget->prevfoc->prev) {}
			if(!widget->prevfoc){
				/* widget is the first focusable widget */
				win->firstfocusable = widget;
			}
		}
		if(!widget->prevfoc){
			/* widget is the first focusable widget */
			win->firstfocusable = widget;
		}
	}
	else {
		struct ui_cbx_info *cbxi = widget->cbx->data;
		if(!cbxi->firstfoc){
			redrawWindow(uiinf, win);
			focus(uiinf, win, widget);
		}
		/*	Find the next focusable widget -
			it has the focusable flag set. */
		for(widget->rightfoc = widget;
			widget->rightfoc && !widget->rightfoc->focusable;
			widget->rightfoc = widget->rightfoc->right) {}
		if(widget->rightfoc){
			widget->leftfoc = widget->rightfoc->leftfoc;
			widget->rightfoc->leftfoc = widget;
		}
		else {
			cbxi->lastfoc = widget;
			/*	Find the previous focusable widget -
				no rightfoc has been found. */
			for(widget->leftfoc = widget;
				widget->leftfoc &&
				!widget->leftfoc->focusable;
				widget->leftfoc = widget->leftfoc->left) {}
		}
		if(!widget->leftfoc){
			/* widget is the first focusable widget */
			cbxi->firstfoc = widget;
		}
	}
	widget->focusable = 1;
}
struct uiinfo *getui()
{
	struct uiinfo *uiinf;
	int i;
	DIR* input_dir;
	struct dirent* input_dirent;
	uiinf = malloc(sizeof(struct uiinfo));
	uiinf->fbinf = getfb();
	uiinf->on = 1;
	uiinf->slidestate = SLIDE_CLOSED;
	handleSlideOpen(uiinf);
	for(i = 0; i < MAX_WINDOWS; i++){
		uiinf->windows[i] = NULL;
	}
	uiinf->nextwindowid = 0;
	input_dir = opendir("/dev/input");
	if(!input_dir){
		fprintf(stderr, "opendir /dev/input failed");
		exit(2);
	}
	for(uiinf->nfds = 0; uiinf->nfds < MAX_INPUTS &&
		(input_dirent = readdir(input_dir)); uiinf->nfds++)
	{
		int id;
		if(0 != strncmp(input_dirent->d_name, "event", 5)){
			uiinf->nfds--;
			continue;
		}
		id = strtol(input_dirent->d_name + 5, NULL, 10);
		uiinf->inpfds[id] = openat(dirfd(input_dir),
			input_dirent->d_name, O_RDONLY);
		if(uiinf->inpfds[id] == -1){
			perror(input_dirent->d_name);
		}
	}
	decompress_font();
	uiinf->slidestate = SLIDE_CLOSED;
	uiinf->listingwindows = 0;
	handleSlideOpen(uiinf);
	return uiinf;
}

void uicleanup(struct uiinfo *uiinf)
{
	fbcleanup(uiinf->fbinf);
}
