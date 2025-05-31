#ifndef DEV_FRAMEBUFFER_H_
#define DEV_FRAMEBUFFER_H_

typedef struct framebuffer_info {
  unsigned int width;
  unsigned int height;
  unsigned int pitch;
  unsigned int isrgb;
} framebuffer_info_t;

void dev_fbuf_init();

#endif // DEV_FRAMEBUFFER_H_