/*
	Graphics library to handle framebuffers.
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
#include "ion.h"
#include "msm_ion.h"
#include "msm_mdp.h"
#define FBDEV "/dev/fb0"


void fbcleanup(struct fbinfo *info)
{
	munmap(info->ion_mem, info->finfo->smem_len);
	if(ioctl(info->iondev_fd, ION_IOC_FREE, &info->ion_handle) < 0){
		perror("ioctl ION_IOC_FREE");
	}
	close(info->fd);
	free(info->framebuffer);
	free(info->finfo);
	free(info->vinfo);
	free(info);
}
void refreshScreen(struct fbinfo *info)
{
	struct mdp_display_commit commit;
	struct msmfb_overlay_data overlay_data;
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
	memset(&overlay_data, 0, sizeof(struct msmfb_overlay_data));
	overlay_data.data.flags = 0;
	overlay_data.data.offset = 0;
	overlay_data.data.memory_id = info->memid;
	overlay_data.id = info->ovid;
	if(ioctl(info->fd, MSMFB_OVERLAY_PLAY, &overlay_data) < 0){
		perror("ioctl MSMFB_OVERLAY_PLAY");
	}
	memset(&commit, 0, sizeof(struct mdp_display_commit));
	commit.flags = MDP_DISPLAY_COMMIT_OVERLAY;
	commit.wait_for_finish = 1;
	memcpy(info->ion_mem, info->framebuffer, info->finfo->line_length *
		info->vinfo->yres);
	if(ioctl(info->fd, MSMFB_DISPLAY_COMMIT, &commit) < 0){
		perror("ioctl MSMFB_DISPLAY_COMMIT");
	}
	/*if(ioctl(info->fd, FBIOPUT_VSCREENINFO, info->vinfo)){
		perror("ioctl FBIOPUT_VSCREENINFO");
	}*/
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
/*void rainbow(struct fbinfo *info)
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
	int fd, iondev_fd;
	int enable = 0;
	unsigned char *framebuffer = (unsigned char*)-1;
	struct fb_fix_screeninfo *finfo;
	struct fb_var_screeninfo *vinfo;
	struct fbinfo *info;
	struct msmfb_overlay_data overlay_data;
	struct mdp_overlay overlay;
	struct mdp_display_commit commit;
	struct ion_fd_data ion_fd;
	struct ion_allocation_data ion_alloc;

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
	printf("%dx%d\n", vinfo->xres, vinfo->yres);
	ion_alloc.flags = 0;
	ion_alloc.len = finfo->line_length * vinfo->yres;
	ion_alloc.align = sysconf(_SC_PAGESIZE);
	ion_alloc.heap_id_mask = ION_HEAP(ION_IOMMU_HEAP_ID) |
		ION_HEAP(21); /* ION_SYSTEM_CONTIG_HEAP_ID */
	iondev_fd = open("/dev/ion", O_RDWR|O_DSYNC|O_CLOEXEC);
	info->iondev_fd = iondev_fd;
	if(iondev_fd < 0){
		perror("/dev/ion");
		exit(1);
	}
	if(ioctl(iondev_fd, ION_IOC_ALLOC,  &ion_alloc) < 0){
		perror("ioctl ION_IOC_ALLOC");
		exit(1);
	}
	ion_fd.handle = ion_alloc.handle;
	info->ion_handle = ion_fd.handle;
	if(ioctl(iondev_fd, ION_IOC_MAP, &ion_fd) < 0){
		perror("ioctl ION_IOC_MAP");
		if(ioctl(iondev_fd, ION_IOC_FREE, &ion_fd.handle) < 0){
			perror("ioctl ION_IOC_FREE");
		}
		exit(1);
	}
	framebuffer = mmap(NULL, finfo->line_length * vinfo->yres,
		PROT_READ | PROT_WRITE,
		MAP_SHARED, ion_fd.fd, 0);
	if(framebuffer == (unsigned char*)-1){
		perror("mmap");
		exit(1);
	}
	memset(&overlay, 0, sizeof(struct mdp_overlay));
	overlay.src.width = 256;
	overlay.src.height = vinfo->yres;
	overlay.src.format = MDP_RGB_565;
	overlay.src_rect.w = vinfo->xres;
	overlay.src_rect.h = vinfo->yres;
	overlay.dst_rect.w = vinfo->xres;
	overlay.dst_rect.h = vinfo->yres;
	overlay.alpha = 0xFF;
	overlay.transp_mask = MDP_TRANSP_NOP;
	overlay.id = MSMFB_NEW_REQUEST;
	if(ioctl(fd, MSMFB_OVERLAY_SET, &overlay) < 0){
		perror("ioctl MSMFB_OVERLAY_SET");
	}
	/*memset(&overlay_data, 0, sizeof(struct msmfb_overlay_data));
	overlay_data.data.flags = 0;
	overlay_data.data.offset = 0;
	overlay_data.data.memory_id = ion_fd.fd;
	overlay_data.id = overlay.id;*/
	info->ovid = overlay.id;
	info->memid = ion_fd.fd;
	/*if(ioctl(fd, MSMFB_OVERLAY_PLAY, &overlay_data) < 0){
		perror("ioctl MSMFB_OVERLAY_PLAY");
		exit(1);
	}
	memset(&commit, 0, sizeof(struct mdp_display_commit));
	commit.flags = MDP_DISPLAY_COMMIT_OVERLAY;
	commit.wait_for_finish = 1;
	if(ioctl(fd, MSMFB_DISPLAY_COMMIT, &commit) < 0){
		perror("ioctl MSMFB_DISPLAY_COMMIT");
	}*/
	info->finfo = finfo;
	info->vinfo = vinfo;
	info->framebuffer = malloc(finfo->line_length * vinfo->yres);
	info->ion_mem = framebuffer;
	info->fd = fd;
	return info;
}
