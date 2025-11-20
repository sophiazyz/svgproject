#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include "../include/svg_render_bmp.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
static void apply_transform(double *x, double *y, Transform *transform) {
    if (!transform) return;

    Transform *current = transform;
    while (current) {
        if (current->rotate_angle != 0) {
            double angle_rad = current->rotate_angle * M_PI / 180.0;
            double cos_angle = cos(angle_rad);
            double sin_angle = sin(angle_rad);
            double new_x = *x * cos_angle - *y * sin_angle;
            double new_y = *x * sin_angle + *y * cos_angle;
            *x = new_x;
            *y = new_y;
        }

        *x += current->translate_x;
        *y += current->translate_y;

        current = current->next;
    }
}

static uint32_t apply_opacity(uint32_t color, double opacity) {
    if (opacity >= 1.0) return color;

    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    r = (uint8_t)(255 * (1 - opacity) + r * opacity);
    g = (uint8_t)(255 * (1 - opacity) + g * opacity);
    b = (uint8_t)(255 * (1 - opacity) + b * opacity);

    return (r << 16) | (g << 8) | b;
}

BitmapCanvas* create_canvas(int width, int height) {
    if (width <= 0 || height <= 0) return NULL;

    BitmapCanvas *canvas = malloc(sizeof(BitmapCanvas));
    canvas->width = width;
    canvas->height = height;
    canvas->pixels = calloc(width * height * 3, sizeof(uint8_t));

    if (!canvas->pixels) {
        free(canvas);
        return NULL;
    }

    memset(canvas->pixels, 0xFF, width * height * 3);
    return canvas;
}

void free_canvas(BitmapCanvas *canvas) {
    if (canvas) {
        free(canvas->pixels);
        free(canvas);
    }
}

static void set_pixel(BitmapCanvas *canvas, int x, int y, uint32_t color) {
    if (x < 0 || x >= canvas->width || y < 0 || y >= canvas->height) return;

    int index = (y * canvas->width + x) * 3;
    canvas->pixels[index] = (color >> 16) & 0xFF;
    canvas->pixels[index + 1] = (color >> 8) & 0xFF;
    canvas->pixels[index + 2] = color & 0xFF;
}

static void draw_circle(BitmapCanvas *canvas, SvgCircle *circle) {
    double cx = circle->cx;
    double cy = circle->cy;
    double r = circle->r;

    apply_transform(&cx, &cy, circle->transform);

    int center_x = (int)cx;
    int center_y = (int)cy;
    int radius = (int)r;

    uint32_t fill_color = apply_opacity(circle->fill_color, circle->opacity);
    uint32_t stroke_color = apply_opacity(circle->stroke_color, circle->opacity);

    for (int y = center_y - radius; y <= center_y + radius; y++) {
        for (int x = center_x - radius; x <= center_x + radius; x++) {
            int dx = x - center_x;
            int dy = y - center_y;
            if (dx * dx + dy * dy <= radius * radius) {
                set_pixel(canvas, x, y, fill_color);
            }
        }
    }

    if (circle->stroke_width > 0 && circle->stroke_color != 0) {
        int stroke_width = (int)circle->stroke_width;
        for (int y = center_y - radius - stroke_width; y <= center_y + radius + stroke_width; y++) {
            for (int x = center_x - radius - stroke_width; x <= center_x + radius + stroke_width; x++) {
                int dx = x - center_x;
                int dy = y - center_y;
                double dist = sqrt(dx * dx + dy * dy);
                if (dist > radius && dist <= radius + stroke_width) {
                    set_pixel(canvas, x, y, stroke_color);
                }
            }
        }
    }
}

static void draw_rect(BitmapCanvas *canvas, SvgRect *rect) {
    double x = rect->x;
    double y = rect->y;
    double width = rect->width;
    double height = rect->height;

    apply_transform(&x, &y, rect->transform);

    int start_x = (int)x;
    int start_y = (int)y;
    int end_x = (int)(x + width);
    int end_y = (int)(y + height);

    uint32_t fill_color = apply_opacity(rect->fill_color, rect->opacity);
    uint32_t stroke_color = apply_opacity(rect->stroke_color, rect->opacity);

    for (int py = start_y; py < end_y; py++) {
        for (int px = start_x; px < end_x; px++) {
            set_pixel(canvas, px, py, fill_color);
        }
    }

    if (rect->stroke_width > 0 && rect->stroke_color != 0) {
        int stroke_width = (int)rect->stroke_width;
        for (int i = 0; i < stroke_width; i++) {
            for (int px = start_x; px < end_x; px++) {
                set_pixel(canvas, px, start_y + i, stroke_color);
                set_pixel(canvas, px, end_y - 1 - i, stroke_color);
            }
            for (int py = start_y; py < end_y; py++) {
                set_pixel(canvas, start_x + i, py, stroke_color);
                set_pixel(canvas, end_x - 1 - i, py, stroke_color);
            }
        }
    }
}

static void draw_rounded_rect(BitmapCanvas *canvas, SvgRoundedRect *rect) {
    draw_rect(canvas, (SvgRect*)rect);
}

static void draw_line(BitmapCanvas *canvas, SvgLine *line) {
    double x1 = line->x1;
    double y1 = line->y1;
    double x2 = line->x2;
    double y2 = line->y2;

    apply_transform(&x1, &y1, line->transform);
    apply_transform(&x2, &y2, line->transform);

    int start_x = (int)x1;
    int start_y = (int)y1;
    int end_x = (int)x2;
    int end_y = (int)y2;

    uint32_t stroke_color = apply_opacity(line->stroke_color, line->opacity);
    int stroke_width = (int)line->stroke_width;

    for (int i = -stroke_width/2; i <= stroke_width/2; i++) {
        int dx = abs(end_x - start_x);
        int dy = abs(end_y - start_y);
        int sx = (start_x < end_x) ? 1 : -1;
        int sy = (start_y < end_y) ? 1 : -1;
        int err = dx - dy;

        int current_x = start_x;
        int current_y = start_y;

        while (1) {
            set_pixel(canvas, current_x, current_y + i, stroke_color);
            if (current_x == end_x && current_y == end_y) break;

            int e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                current_x += sx;
            }
            if (e2 < dx) {
                err += dx;
                current_y += sy;
            }
        }
    }
}

void render_svg_to_bitmap(SvgDocument *doc, BitmapCanvas **canvas_out) {
    if (!doc || !canvas_out) return;

    int width = (int)doc->width;
    int height = (int)doc->height;

    if (width <= 0 || height <= 0) {
        width = 500;
        height = 300;
    }

    BitmapCanvas *canvas = create_canvas(width, height);
    if (!canvas) return;

    SvgShape *current = doc->shapes;
    while (current) {
        switch (current->type) {
            case SVG_SHAPE_CIRCLE: draw_circle(canvas, &current->data.circle); break;
            case SVG_SHAPE_RECT: draw_rect(canvas, &current->data.rect); break;
            case SVG_SHAPE_LINE: draw_line(canvas, &current->data.line); break;
        }
        current = current->next;
    }

    *canvas_out = canvas;
}