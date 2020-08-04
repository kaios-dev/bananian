/*
	Small graphics library to handle framebuffers.
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
#include <linux/fb.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gr.h"
#define FBDEV "/dev/fb0"


void fbcleanup(struct fbinfo *info)
{
	munmap(info->framebuffer, info->finfo->smem_len);
	close(info->fd);
	free(info->finfo);
	free(info->vinfo);
	free(info);
}
void refreshScreen(struct fbinfo *info)
{
	/*if(info->vinfo->yoffset > 0){
		info->vinfo->yoffset = 0;
		memcpy(info->framebuffer + (info->finfo->smem_len / 2),
			info->framebuffer, info->finfo->smem_len / 2);
		info->framebuffer += (info->finfo->smem_len / 2);
	}
	else {
		info->vinfo->yoffset = info->vinfo->yres;
		memcpy(info->framebuffer - (info->finfo->smem_len / 2),
			info->framebuffer, info->finfo->smem_len / 2);
		info->framebuffer -= (info->finfo->smem_len / 2);
	}*/
	memcpy(info->real_framebuffer, info->framebuffer, info->vinfo->yres *
		info->finfo->line_length);
	info->vinfo->activate = FB_ACTIVATE_VBL;
	if(ioctl(info->fd, FBIOPAN_DISPLAY, info->vinfo)){
		perror("ioctl FBIOPAN_DISPLAY");
	}
}
void setPixel(struct fbinfo *info, int y, int x, int r, int g, int b, int a)
{
	int val;
	unsigned char *fb;
	if(y > info->vinfo->yres || x > info->vinfo->xres) return;
	r >>= 5;
	g >>= 5;
	b >>= 6;
	fb = info->framebuffer;
	val = ((r & 7) << 5) | ((b & 3) << 3) | (g & 7);
	fb[y*info->finfo->line_length + x*2 + 0] =
		(fb[y*info->finfo->line_length + x*2 + 0] * (255-a) +
		val * a) / 255;
	fb[y*info->finfo->line_length + x*2 + 1] =
		(fb[y*info->finfo->line_length + x*2 + 1] * (255-a) +
		val * a) / 255;
}

void drawRect(struct fbinfo *info, int y, int x, int h, int w, int r,
	int g, int b, int a)
{
	int i, j;
	for(i = y; i < y + h; i++){
		if(i + h < 0) break;
		if(i > 0 && i >= info->vinfo->yres) break;
		for(j = x; j < x + w; j++){
			if(j + w < 0) break;
			if(j > 0 && j >= info->vinfo->xres) break;
			setPixel(info, i, j, r, g, b, a);
		}
	}
}
/*void doWork(struct fbinfo *info)
{
	int r=0, g=0, b=0, i=0;
	for(; r < 7; r++, i++){
		drawRect(info, i*5, 10, 5, 100, r, g, b);
	}
	for(; g < 7; g++, i++){
		drawRect(info, i*5, 10, 5, 100, r, g, b);
	}
	for(; r > 0; r--, i++){
		drawRect(info, i*5, 10, 5, 100, r, g, b);
	}
	for(; b < 3; b++, i++){
		drawRect(info, i*5, 10, 5, 100, r, g, b);
	}
	for(; g > 0; g--, i++){
		drawRect(info, i*5, 10, 5, 100, r, g, b);
	}
	for(; r < 7; r++, i++){
		drawRect(info, i*5, 10, 5, 100, r, g, b);
	}
	for(; r > 0; r--, b = b ? b-1 : 0, i++){
		drawRect(info, i*5, 10, 5, 100, r, g, b);
	}
	refreshScreen(info);
}

int main()
{*/
struct fbinfo *getfb(){
	int fd;
	unsigned char *framebuffer = (unsigned char*)-1;
	struct fb_fix_screeninfo *finfo;
	struct fb_var_screeninfo *vinfo;
	struct fbinfo *info;
	finfo = malloc(sizeof(struct fb_fix_screeninfo));
	vinfo = malloc(sizeof(struct fb_var_screeninfo));
	info = malloc(sizeof(struct fbinfo));
	fd = open(FBDEV, O_RDWR);
	if(fd < 0){
		perror(FBDEV);
		exit(2);
	}
	if(ioctl(fd, FBIOGET_FSCREENINFO, finfo) < 0){
		perror("ioctl FBIOGET_FSCREENINFO");
		exit(1);
	}
	if(ioctl(fd, FBIOGET_VSCREENINFO, vinfo) < 0){
		perror("ioctl FBIOGET_VSCREENINFO");
		exit(1);
	}
#ifndef DESKTOP
	if(ioctl(fd, FBIOBLANK, FB_BLANK_POWERDOWN) < 0){
		perror("ioctl FBIOBLANK");
	}
	sleep(1);
	if(ioctl(fd, FBIOBLANK, FB_BLANK_UNBLANK) < 0){
		perror("ioctl FBIOBLANK_UNBLANK");
	}
#endif
	printf("type: 0x%x\n", finfo->type);
	printf("visual: %d\n", finfo->visual);
	printf("line_length: %d\n", finfo->line_length);
	printf("%dx%d\n", vinfo->xres, vinfo->yres);
	framebuffer = mmap(0, finfo->smem_len,
		PROT_READ | PROT_WRITE,
		MAP_SHARED, fd, 0);
	if(framebuffer == (unsigned char*)-1){
		perror("mmap");
		exit(1);
	}
	memset(framebuffer, 0, finfo->smem_len);
	info->finfo = finfo;
	info->vinfo = vinfo;
	info->real_framebuffer = framebuffer;
	info->framebuffer = malloc(vinfo->yres * finfo->line_length);
	info->fd = fd;
	return info;
}
