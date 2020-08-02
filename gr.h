#ifndef _GR_H_
#define _GR_H_
#include <linux/fb.h>

struct fbinfo {
	unsigned char *framebuffer, *real_framebuffer;
	struct fb_fix_screeninfo *finfo;
	struct fb_var_screeninfo *vinfo;
	int fd;
};

#define fbcpy(__dst, __src, __yoffset) memcpy((__dst)->framebuffer + \
	((__dst)->finfo->line_length * (__yoffset)), (__src), \
	((__dst)->finfo->line_length * ((__dst)->vinfo->yres - \
		(__yoffset))))

void fbcleanup(struct fbinfo *info);
void refreshScreen(struct fbinfo *info);
void setPixel(struct fbinfo *info, int y, int x, int r, int g, int b,
	int a);
void drawRect(struct fbinfo *info, int y, int x, int h, int w, int r,
	int g, int b, int a);
struct fbinfo *getfb();

#endif
