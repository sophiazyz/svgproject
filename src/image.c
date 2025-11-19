#include <stdlib.h>

#include "../include/image.h"

Image* create_image(int width, int height) {
    Image *img = malloc(sizeof(Image));
    img->width = width;
    img->height = height;
    img->pixels = calloc(width * height, sizeof(RGBColor));
    return img;
}

void free_image(Image *img) {
    if (img) {
        free(img->pixels);
        free(img);
    }
}

void set_pixel(Image *img, int x, int y, RGBColor color) {
    if (x >= 0 && x < img->width && y >= 0 && y < img->height) {
        img->pixels[y * img->width + x] = color;
    }
}