#ifndef IMAGE_H
#define IMAGE_H

typedef struct {
    unsigned char r, g, b;
} RGBColor;

typedef struct {
    int width, height;
    RGBColor *pixels;
} Image;

Image* create_image(int width, int height);
void free_image(Image *img);
void set_pixel(Image *img, int x, int y, RGBColor color);

#endif