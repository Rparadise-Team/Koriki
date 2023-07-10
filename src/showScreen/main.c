#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

int main(int argc , char* argv[]) {
	if (argc<2) {
		puts("Usage: showScreen /path/image.png");
		return EXIT_SUCCESS;
	}
		
	if (access(argv[1], F_OK) != 0) return EXIT_FAILURE;
		
	int fb0_fd = open("/dev/fb0", O_RDWR);
	struct fb_var_screeninfo vinfo;
	ioctl(fb0_fd, FBIOGET_VSCREENINFO, &vinfo);
	int map_size = vinfo.xres * vinfo.yres * (vinfo.bits_per_pixel / 8);
	char* fb0_map = (char*)mmap(0, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb0_fd, 0);
	memset(fb0_map, 0, map_size);
	
	SDL_Surface* img = IMG_Load(argv[1]);
	
	uint8_t* dst = (uint8_t*)fb0_map;
	uint8_t* src = (uint8_t*)img->pixels;
	src += ((img->h * img->w) - 1) * 3;
	for (int y=0; y<img->h; y++) {
		for (int x=0; x<img->w; x++) {
			*(dst+0) = *(src+2);
			*(dst+1) = *(src+1);
			*(dst+2) = *(src+0);
			*(dst+3) = 0xf;
			dst += 4;
			src -= 3;
		}
	}
	SDL_FreeSurface(img);
	
	munmap(fb0_map, map_size);
	close(fb0_fd);
	
	return EXIT_SUCCESS;
}