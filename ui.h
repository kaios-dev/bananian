#ifndef _UI_H_
#define _UI_H_
#define MAX_WINDOWS 128
#define MAX_WIDGETS 256
#define MAX_INPUTS 16
#define TITLE_SIZE 32
#define SK_LABEL_SIZE 16
#include <linux/input.h>
#include "gr.h"

enum ui_widget_type { UI_LABEL, UI_BUTTON, UI_INPUT, UI_FOBUTTON,
	UI_FOINPUT, UI_CBX, UI_IMAGE };
enum ui_text_align { UI_TA_LEFT, UI_TA_CENTER, UI_TA_RIGHT };
enum ui_event { UI_EVENT_CLICK, UI_EVENT_KEYUP, UI_EVENT_KEYDOWN,
	UI_EVENT_EXIT };
enum slide_state { SLIDE_CLOSED, SLIDE_HALFOPEN, SLIDE_FULLOPEN };

typedef void (*fd_callback)(void*, int);

#define from_rgb(r, g, b) ((r) << 24 | (g) << 16 | (b) << 8 | 255)
#define from_rgba(r, g, b, a) ((r) << 24 | (g) << 16 | (b) << 8 | (a))

static int get_red(unsigned int color)
{
	return color >> 24;
}
static int get_green(unsigned int color)
{
	return color >> 16 & 255;
}
static int get_blue(unsigned int color)
{
	return color >> 8 & 255;
}
static int get_alpha(unsigned int color)
{
	return color & 255;
}

struct ui_style {
	unsigned int fgcolor;
	unsigned int bgcolor;
	unsigned int focolor;
};

struct ui_widget {
	enum ui_widget_type type;
	enum ui_text_align textalign;
	int id, focusable, freedata;
	int x, y, rightend, height;
	void *data;
	struct ui_widget *next;
	struct ui_widget *prev;
	struct ui_widget *right;
	struct ui_widget *left;
	struct ui_widget *nextfoc;
	struct ui_widget *prevfoc;
	struct ui_widget *leftfoc;
	struct ui_widget *rightfoc;
	struct ui_widget *cbx;
	struct ui_style style;
};

typedef void (*event_callback)(void*, int, enum ui_event);

struct ui_window {
	struct ui_widget* widgets[MAX_WIDGETS];
	struct ui_window *parent;
	struct ui_widget *firstfocusable;
	struct ui_widget *lastfocusable;
	struct ui_widget *firstwidget;
	struct ui_widget *lastwidget;
	struct ui_widget *focused;
	int skbgr, skbgg, skbgb, skbga;
	int skfgr, skfgg, skfgb;
	int bgr, bgg, bgb;
	char title[TITLE_SIZE];
	char current_skl[SK_LABEL_SIZE];
	char current_skc[SK_LABEL_SIZE];
	char current_skr[SK_LABEL_SIZE];
	int nextwidgetid, id, cbx_focus_index;
	int scrolloffset, height, yoffset, actual_yoffset, maxscroll;
	event_callback event;
	void *userdata;
	unsigned int hidetoppanel	: 1;
	unsigned int hideskpanel	: 1;
};

struct uiinfo {
	struct fbinfo *fbinf;
	struct ui_window* windows[MAX_WINDOWS];
	struct ui_window *curwindow;
	int nextwindowid, nfds, inpfds[MAX_INPUTS], on, listingwindows;
	enum slide_state slidestate;
};

struct ui_cbx_info {
	struct ui_widget *first;
	struct ui_widget *last;
	struct ui_widget *firstfoc;
	struct ui_widget *lastfoc;
	int numwidgets;
};

void redrawWidget(struct uiinfo *uiinf, struct ui_window *win,
	struct ui_widget *widget);
void focus(struct uiinfo *uiinf, struct ui_window *win,
	struct ui_widget *widget);
struct ui_window *createWindow(struct uiinfo *uiinf);
void destroyWindow(struct uiinfo *uiinf, struct ui_window *win);
void clearWindow(struct uiinfo *uiinf, struct ui_window *win);
void redrawWindow(struct uiinfo *uiinf, struct ui_window *win);
void addWidget(struct uiinfo *uiinf, struct ui_window *win,
	struct ui_widget *widget, int focusable);
void uiLoop(struct uiinfo *uiinf, int *custom_fds, int *ncustom_fds,
	fd_callback custom_callback, void *userdata);
void makeFocusable(struct uiinfo *uiinf, struct ui_window *win,
	struct ui_widget *widget);
void makeUnfocusable(struct uiinfo *uiinf, struct ui_window *win,
	struct ui_widget *widget);
void addWidgetToCbx(struct uiinfo *uiinf, struct ui_window *win,
	struct ui_widget *cbx, struct ui_widget *widget, int focusable);
struct uiinfo *getui();
void uicleanup(struct uiinfo *uiinf);

#endif
