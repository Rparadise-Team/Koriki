#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

int main(int argc , char* argv[]) {
    if (argc < 2) {
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
    int src_pitch = img->pitch;
    int dst_pitch = vinfo.xres * (vinfo.bits_per_pixel / 8);
    
    int src_x, src_y, dst_x, dst_y;
    int x_ratio = (img->w << 16) / vinfo.xres;
    int y_ratio = (img->h << 16) / vinfo.yres;
    
    for (dst_y = 0; (unsigned int)dst_y < vinfo.yres; dst_y++) {
        src_y = ((vinfo.yres - dst_y - 1) * y_ratio) >> 16;
        for (dst_x = 0; (unsigned int)dst_x < vinfo.xres; dst_x++) {
            src_x = ((vinfo.xres - dst_x - 1) * x_ratio) >> 16;
            uint8_t* pixel = src + src_y * src_pitch + src_x * img->format->BytesPerPixel;
            
            uint8_t blue = *(pixel + 0);
            uint8_t green = *(pixel + 1);
            uint8_t red = *(pixel + 2);
            
            uint32_t color = (blue << 16) | (green << 8) | red;
            
            *(uint32_t*)(dst + dst_y * dst_pitch + dst_x * 4) = color;
        }
    }
    
    SDL_FreeSurface(img);
    
    munmap(fb0_map, map_size);
    close(fb0_fd);
    
    return EXIT_SUCCESS;
}
