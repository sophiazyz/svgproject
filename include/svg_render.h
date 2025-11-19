#ifndef SVG_RENDER_H
#define SVG_RENDER_H

#include "image.h"

typedef struct {
    char type[20];
    float x, y, x2, y2, cx, cy, r, width, height;
    RGBColor color;
} SVGShape;

RGBColor parse_color(const char *color_str);
int parse_svg(const char *filename, SVGShape **shapes, int *shape_count);
void draw_line(Image *img, float x1, float y1, float x2, float y2, RGBColor color);
void draw_rectangle(Image *img, float x, float y, float width, float height, RGBColor color);
void draw_circle(Image *img, float cx, float cy, float r, RGBColor color);
void render_svg_to_image(Image *img, SVGShape *shapes, int shape_count);

#endif