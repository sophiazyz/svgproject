#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/svg_gui_types.h"
#include "../include/image.h"
#include "../include/bmp_writer.h"
#include "../include/jpg_writer.h"

// 创建SVG文档
SvgDocument* create_svg_document(double width, double height) {
    SvgDocument* doc = (SvgDocument*)malloc(sizeof(SvgDocument));
    if (!doc) return NULL;

    doc->width = width;
    doc->height = height;
    doc->shape_count = 0;

    return doc;
}

// 解析颜色字符串
unsigned int parse_color(const char* color_str) {
    if (!color_str) return 0x000000;

    // 如果是十六进制格式
    if (color_str[0] == '#') {
        unsigned int color = 0;
        sscanf(color_str + 1, "%06x", &color);
        return color;
    }

    // 预定义颜色
    if (strcmp(color_str, "red") == 0) return 0xFF0000;
    if (strcmp(color_str, "green") == 0) return 0x00FF00;
    if (strcmp(color_str, "blue") == 0) return 0x0000FF;
    if (strcmp(color_str, "yellow") == 0) return 0xFFFF00;
    if (strcmp(color_str, "orange") == 0) return 0xFFA500;
    if (strcmp(color_str, "purple") == 0) return 0x800080;
    if (strcmp(color_str, "pink") == 0) return 0xFFC0CB;
    if (strcmp(color_str, "black") == 0) return 0x000000;
    if (strcmp(color_str, "white") == 0) return 0xFFFFFF;
    if (strcmp(color_str, "gray") == 0) return 0x808080;

    return 0x000000; // 默认黑色
}

// 转换颜色为RGB结构
void color_to_rgb(unsigned int color, int* r, int* g, int* b) {
    *r = (color >> 16) & 0xFF;
    *g = (color >> 8) & 0xFF;
    *b = color & 0xFF;
}

// 渲染SVG到Image
int render_svg_to_image(SvgDocument* doc, Image* img) {
    if (!doc || !img) return 0;

    // 初始化图像为白色背景
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            img->pixels[y * img->width + x].r = 255;
            img->pixels[y * img->width + x].g = 255;
            img->pixels[y * img->width + x].b = 255;
        }
    }

    // 渲染所有图形
    for (int i = 0; i < doc->shape_count; i++) {
        SvgShape* shape = &doc->shapes[i];
        unsigned int color = 0;
        int r, g, b;

        switch (shape->type) {
            case SVG_SHAPE_CIRCLE:
                color = parse_color(shape->data.circle.fill);
                color_to_rgb(color, &r, &g, &b);

                // 简单的圆形绘制算法（中点圆算法）
                {
                    int cx = (int)shape->data.circle.cx;
                    int cy = (int)shape->data.circle.cy;
                    int radius = (int)shape->data.circle.r;

                    for (int y = -radius; y <= radius; y++) {
                        for (int x = -radius; x <= radius; x++) {
                            if (x*x + y*y <= radius*radius) {
                                int px = cx + x;
                                int py = cy + y;
                                if (px >= 0 && px < img->width && py >= 0 && py < img->height) {
                                    img->pixels[py * img->width + px].r = r;
                                    img->pixels[py * img->width + px].g = g;
                                    img->pixels[py * img->width + px].b = b;
                                }
                            }
                        }
                    }
                }
                break;

            case SVG_SHAPE_RECT:
                color = parse_color(shape->data.rect.fill);
                color_to_rgb(color, &r, &g, &b);

                // 绘制矩形
                {
                    int x1 = (int)shape->data.rect.x;
                    int y1 = (int)shape->data.rect.y;
                    int x2 = x1 + (int)shape->data.rect.width;
                    int y2 = y1 + (int)shape->data.rect.height;

                    for (int y = y1; y < y2 && y < img->height; y++) {
                        if (y < 0) continue;
                        for (int x = x1; x < x2 && x < img->width; x++) {
                            if (x < 0) continue;
                            img->pixels[y * img->width + x].r = r;
                            img->pixels[y * img->width + x].g = g;
                            img->pixels[y * img->width + x].b = b;
                        }
                    }
                }
                break;

            case SVG_SHAPE_LINE:
                color = parse_color(shape->data.line.stroke);
                color_to_rgb(color, &r, &g, &b);

                // Bresenham直线算法
                {
                    int x1 = (int)shape->data.line.x1;
                    int y1 = (int)shape->data.line.y1;
                    int x2 = (int)shape->data.line.x2;
                    int y2 = (int)shape->data.line.y2;

                    int dx = abs(x2 - x1);
                    int dy = abs(y2 - y1);
                    int sx = x1 < x2 ? 1 : -1;
                    int sy = y1 < y2 ? 1 : -1;
                    int err = dx - dy;

                    while (1) {
                        if (x1 >= 0 && x1 < img->width && y1 >= 0 && y1 < img->height) {
                            img->pixels[y1 * img->width + x1].r = r;
                            img->pixels[y1 * img->width + x1].g = g;
                            img->pixels[y1 * img->width + x1].b = b;
                        }

                        if (x1 == x2 && y1 == y2) break;

                        int e2 = 2 * err;
                        if (e2 > -dy) {
                            err -= dy;
                            x1 += sx;
                        }
                        if (e2 < dx) {
                            err += dx;
                            y1 += sy;
                        }
                    }
                }
                break;
        }
    }

    return 1;
}

// 导出到BMP
int export_to_bmp(SvgDocument* doc, const char* filename) {
    if (!doc || !filename) return 0;

    // 创建图像
    Image img;
    img.width = (int)doc->width;
    img.height = (int)doc->height;
    img.pixels = (RGBColor*)malloc(img.width * img.height * sizeof(RGBColor));
    if (!img.pixels) return 0;

    // 渲染SVG到图像
    if (!render_svg_to_image(doc, &img)) {
        free(img.pixels);
        return 0;
    }

    // 保存为BMP
    write_bmp(filename, &img);
    int result = 1; // 假设成功

    // 清理
    free(img.pixels);

    return result;
}

// 导出到JPG
int export_to_jpg(SvgDocument* doc, const char* filename) {
    if (!doc || !filename) return 0;

    // 创建图像
    Image img;
    img.width = (int)doc->width;
    img.height = (int)doc->height;
    img.pixels = (RGBColor*)malloc(img.width * img.height * sizeof(RGBColor));
    if (!img.pixels) return 0;

    // 渲染SVG到图像
    if (!render_svg_to_image(doc, &img)) {
        free(img.pixels);
        return 0;
    }

    // 保存为JPG
    write_jpg(filename, &img, 90); // 90% quality
    int result = 1; // 假设成功

    // 清理
    free(img.pixels);

    return result;
}