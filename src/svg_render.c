#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "../include/svg_render.h"
#include "../include/image.h"

//-------- 工具函数：读取完整标签 --------//
static void read_full_tag(FILE *fp, const char *first_line, char *out_tag)
{
    strcpy(out_tag, first_line);

    while (!strchr(out_tag, '>')) {
        char buf[2048];
        if (!fgets(buf, sizeof(buf), fp)) break;
        strcat(out_tag, buf);
    }
}

//-------- 工具函数：跳过空白 --------//
static void skip_spaces(const char **p)
{
    while (isspace((unsigned char)**p))
        (*p)++;
}


// 解析十六进制颜色和 rgb() 颜色
RGBColor parse_color(const char *color_str) {
    RGBColor color = {0, 0, 0};
    /*test
    printf("解析颜色字符串: %s\n", color_str);
    */
    if (color_str[0] == '#') {
        /*test
        printf("进入if条件\n");
        */
        int r, g, b;
        if (strlen(color_str) == 7) {
            sscanf(color_str, "#%02x%02x%02x", &r, &g, &b);
        } else if (strlen(color_str) == 4) {
            sscanf(color_str, "#%1x%1x%1x", &r, &g, &b);
            r *= 17; g *= 17; b *= 17;
        }
        color.r = r;
        color.g = g;
        color.b = b;
        /*test
        printf("  解析为 rgb() 颜色:%d,%d,%d\n", color.r, color.g, color.b);
        */
    } else if (strstr(color_str, "rgb(")) {
        /*test
        printf("进入else if 条件\n");
        */
        int r, g, b;
        sscanf(color_str, "rgb(%d,%d,%d)", &r, &g, &b);
        color.r = r;
        color.g = g;
        color.b = b;
        /*test
        printf("  解析为 rgb() 颜色:%d,%d,%d\n", color.r, color.g, color.b);
        */
    } else {
        printf("进入else 条件\n");
        // 处理常见颜色名称
        const char *colors[][2] = {
            {"red", "#ff0000"}, {"green", "#00ff00"}, {"blue", "#0000ff"},
            {"black", "#000000"}, {"white", "#ffffff"}, {"yellow", "#ffff00"},
            {"cyan", "#00ffff"}, {"magenta", "#ff00ff"}, {"gray", "#808080"}
        };
        /*test
        printf("  解析为颜色名称\n");
        */
        for (int i = 0; i < sizeof(colors)/sizeof(colors[0]); i++) {
            if (strcmp(color_str, colors[i][0]) == 0) {
                return parse_color(colors[i][1]);
            }
        }
    }
    return color;
}

// 解析 SVG 文件，提取形状信息
int parse_svg(const char *filename, SVGShape **shapes, int *shape_count) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("无法打开文件: %s\n", filename);
        return 0;
    }
    
    char line[1024];
    *shape_count = 0;
    int capacity = 10;
    *shapes = malloc(capacity * sizeof(SVGShape));
    
    while (fgets(line, sizeof(line), file)) {
        // 去掉前后空白
        const char *p = line;
        skip_spaces(&p);

        char tag[4096];
        read_full_tag(file, line, tag);

        char *trimmed = tag;
        while (*trimmed == ' ' || *trimmed == '\t') trimmed++;

        /* test
            printf("%s\n", trimmed);
        */
        if (strstr(trimmed, "<rect")) {
            if (*shape_count >= capacity) {
                capacity *= 2;
                *shapes = realloc(*shapes, capacity * sizeof(SVGShape));
            }
            
            SVGShape *shape = &(*shapes)[(*shape_count)++];
            strcpy(shape->type, "rect");
            
            char *x = strstr(trimmed, "x=\"");
            char *y = strstr(trimmed, "y=\"");
            char *width = strstr(trimmed, "width=\"");
            char *height = strstr(trimmed, "height=\"");
            char *fill = strstr(trimmed, "fill=\"");
            /*test
            printf("%c\n", x ? 'T' : 'F');
            printf("%c\n", y ? 'T' : 'F');
            printf("%c\n", width ? 'T' : 'F');
            printf("%c\n", height ? 'T' : 'F');
            printf("%c\n", fill ? 'T' : 'F');
            */
            
            if (x) sscanf(x, "x=\"%f\"", &shape->x);
            if (y) sscanf(y, "y=\"%f\"", &shape->y);
            if (width) sscanf(width, "width=\"%f\"", &shape->width);
            if (height) sscanf(height, "height=\"%f\"", &shape->height);
            if (fill) {
                /*test
                printf("进入if(fill)条件\n");
                */
                char color_str[50];
                sscanf(fill, "fill=\"%49[^\"]\"", color_str);
                shape->color = parse_color(color_str);
            } else {
                /*test
                printf("进入else(fill)条件\n");
                */
                shape->color = parse_color("black");
            }
        }
        else if (strstr(trimmed, "<circle")) {
            if (*shape_count >= capacity) {
                capacity *= 2;
                *shapes = realloc(*shapes, capacity * sizeof(SVGShape));
            }
            
            SVGShape *shape = &(*shapes)[(*shape_count)++];
            strcpy(shape->type, "circle");
            
            char *cx = strstr(trimmed, "cx=\"");
            char *cy = strstr(trimmed, "cy=\"");
            char *r = strstr(trimmed, "r=\"");
            char *fill = strstr(trimmed, "fill=\"");
            
            if (cx) sscanf(cx, "cx=\"%f\"", &shape->cx);
            if (cy) sscanf(cy, "cy=\"%f\"", &shape->cy);
            if (r) sscanf(r, "r=\"%f\"", &shape->r);
            if (fill) {
                char color_str[50];
                sscanf(fill, "fill=\"%49[^\"]\"", color_str);
                shape->color = parse_color(color_str);
            } else {
                shape->color = parse_color("black");
            }
        }
        else if (strstr(trimmed, "<line")) {
            if (*shape_count >= capacity) {
                capacity *= 2;
                *shapes = realloc(*shapes, capacity * sizeof(SVGShape));
            }
            
            SVGShape *shape = &(*shapes)[(*shape_count)++];
            strcpy(shape->type, "line");
            
            char *x1 = strstr(trimmed, "x1=\"");
            char *y1 = strstr(trimmed, "y1=\"");
            char *x2 = strstr(trimmed, "x2=\"");
            char *y2 = strstr(trimmed, "y2=\"");
            char *stroke = strstr(trimmed, "stroke=\"");
            
            if (x1) sscanf(x1, "x1=\"%f\"", &shape->x);
            if (y1) sscanf(y1, "y1=\"%f\"", &shape->y);
            if (x2) sscanf(x2, "x2=\"%f\"", &shape->x2);
            if (y2) sscanf(y2, "y2=\"%f\"", &shape->y2);
            if (stroke) {
                char color_str[50];
                sscanf(stroke, "stroke=\"%49[^\"]\"", color_str);
                shape->color = parse_color(color_str);
            } else {
                shape->color = parse_color("black");
            }
        }
    }
    
    fclose(file);
    return 1;
}

// 绘制线条
void draw_line(Image *img, float x1, float y1, float x2, float y2, RGBColor color) {
    int ix1 = (int)x1, iy1 = (int)y1;
    int ix2 = (int)x2, iy2 = (int)y2;
    
    int dx = abs(ix2 - ix1);
    int dy = abs(iy2 - iy1);
    int sx = (ix1 < ix2) ? 1 : -1;
    int sy = (iy1 < iy2) ? 1 : -1;
    int err = dx - dy;
    
    while (1) {
        set_pixel(img, ix1, iy1, color);
        
        if (ix1 == ix2 && iy1 == iy2) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            ix1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            iy1 += sy;
        }
    }
}

// 绘制矩形和圆形
void draw_rectangle(Image *img, float x, float y, float width, float height, RGBColor color) {
    int ix = (int)x, iy = (int)y;
    int iw = (int)width, ih = (int)height;
    
    for (int py = iy; py < iy + ih; py++) {
        for (int px = ix; px < ix + iw; px++) {
            set_pixel(img, px, py, color);
        }
    }
}

// 绘制圆形
void draw_circle(Image *img, float cx, float cy, float r, RGBColor color) {
    int icx = (int)cx, icy = (int)cy;
    int ir = (int)r;
    
    for (int y = -ir; y <= ir; y++) {
        for (int x = -ir; x <= ir; x++) {
            if (x*x + y*y <= ir*ir) {
                set_pixel(img, icx + x, icy + y, color);
            }
        }
    }
}

// 渲染 SVG 形状到图像
void render_svg_to_image(Image *img, SVGShape *shapes, int shape_count) {
    // 初始化白色背景
    RGBColor white = {255, 255, 255};
    for (int i = 0; i < img->width * img->height; i++) {
        img->pixels[i] = white;
    }
    
    // 绘制所有形状
    for (int i = 0; i < shape_count; i++) {
        SVGShape *shape = &shapes[i];
        
        /*打印调试信息
        printf("绘制形状 %d: 类型=%s, 颜色=(%d, %d, %d)\n", i, shape->type, shape->color.r, shape->color.g, shape->color.b);
        
        if (strcmp(shape->type, "rect") == 0) {
            printf("  矩形: x=%.2f, y=%.2f, width=%.2f, height=%.2f\n", shape->x, shape->y, shape->width, shape->height);
            draw_rectangle(img, shape->x, shape->y, shape->width, shape->height, shape->color);
        } else if (strcmp(shape->type, "circle") == 0) {
            printf("  圆形: cx=%.2f, cy=%.2f, r=%.2f\n", shape->cx, shape->cy, shape->r);
            draw_circle(img, shape->cx, shape->cy, shape->r, shape->color);
        } else if (strcmp(shape->type, "line") == 0) {
            printf("  直线: x1=%.2f, y1=%.2f, x2=%.2f, y2=%.2f\n", shape->x, shape->y, shape->x2, shape->y2);
            draw_line(img, shape->x, shape->y, shape->x2, shape->y2, shape->color);
        }
        */
        
        if (strcmp(shape->type, "rect") == 0) {
            draw_rectangle(img, shape->x, shape->y, shape->width, shape->height, shape->color);
        } else if (strcmp(shape->type, "circle") == 0) {
            draw_circle(img, shape->cx, shape->cy, shape->r, shape->color);
        } else if (strcmp(shape->type, "line") == 0) {
            draw_line(img, shape->x, shape->y, shape->x2, shape->y2, shape->color);
        }
        
    }
}

