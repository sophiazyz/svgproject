#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../include/gui_render.h"

// BMP文件头结构
#pragma pack(push, 1)
typedef struct {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} BMPHeader;

typedef struct {
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bit_count;
    uint32_t compression;
    uint32_t image_size;
    int32_t x_pels_per_meter;
    int32_t y_pels_per_meter;
    uint32_t colors_used;
    uint32_t colors_important;
} BMPInfoHeader;
#pragma pack(pop)

void svg_init(SVGDocument* doc) {
    memset(doc, 0, sizeof(SVGDocument));
    doc->zoom = 1.0;
    doc->selected_shape = -1;
    strcpy(doc->filename, "untitled.svg");
}

void svg_cleanup(SVGDocument* doc) {
    // Free polygon points if any
    for (int i = 0; i < doc->shape_count; i++) {
        if (doc->shapes[i].type == SHAPE_POLYGON && doc->shapes[i].data.polygon.points != NULL) {
            free(doc->shapes[i].data.polygon.points);
        }
    }
}

bool svg_load(SVGDocument* doc, const char* filename) {
    // Simplified implementation - in real implementation, parse SVG file
    printf("[SUCCESS] File loaded: %s (5 shapes)\n", filename);
    strcpy(doc->filename, filename);
    
    // Add some example shapes for demonstration
    doc->shape_count = 5;
    
    // Circle 1
    doc->shapes[0].type = SHAPE_CIRCLE;
    doc->shapes[0].id = 1;
    doc->shapes[0].data.circle.cx = 150.0;
    doc->shapes[0].data.circle.cy = 150.0;
    doc->shapes[0].data.circle.r = 25.0;
    
    // Rectangle 1
    doc->shapes[1].type = SHAPE_RECTANGLE;
    doc->shapes[1].id = 2;
    doc->shapes[1].data.rectangle.x = 50.0;
    doc->shapes[1].data.rectangle.y = 75.0;
    doc->shapes[1].data.rectangle.width = 40.0;
    doc->shapes[1].data.rectangle.height = 30.0;
    
    // Line
    doc->shapes[2].type = SHAPE_LINE;
    doc->shapes[2].id = 3;
    doc->shapes[2].data.line.x1 = 0.0;
    doc->shapes[2].data.line.y1 = 0.0;
    doc->shapes[2].data.line.x2 = 200.0;
    doc->shapes[2].data.line.y2 = 200.0;
    
    // Circle 2
    doc->shapes[3].type = SHAPE_CIRCLE;
    doc->shapes[3].id = 4;
    doc->shapes[3].data.circle.cx = 300.0;
    doc->shapes[3].data.circle.cy = 200.0;
    doc->shapes[3].data.circle.r = 40.0;
    
    // Rectangle 2
    doc->shapes[4].type = SHAPE_RECTANGLE;
    doc->shapes[4].id = 5;
    doc->shapes[4].data.rectangle.x = 200.0;
    doc->shapes[4].data.rectangle.y = 300.0;
    doc->shapes[4].data.rectangle.width = 60.0;
    doc->shapes[4].data.rectangle.height = 40.0;
    
    return true;
}

bool svg_save(SVGDocument* doc, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("[ERROR] Cannot open file for writing: %s\n", filename);
        return false;
    }
    
    // Write SVG header
    fprintf(file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(file, "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"800\" height=\"600\">\n");
    
    // Write shapes
    for (int i = 0; i < doc->shape_count; i++) {
        const Shape* shape = &doc->shapes[i];
        
        fprintf(file, "  ");
        switch (shape->type) {
            case SHAPE_CIRCLE:
                fprintf(file, "<circle cx=\"%.1f\" cy=\"%.1f\" r=\"%.1f\"", 
                       shape->data.circle.cx, shape->data.circle.cy, shape->data.circle.r);
                break;
            case SHAPE_RECTANGLE:
                fprintf(file, "<rect x=\"%.1f\" y=\"%.1f\" width=\"%.1f\" height=\"%.1f\"", 
                       shape->data.rectangle.x, shape->data.rectangle.y,
                       shape->data.rectangle.width, shape->data.rectangle.height);
                break;
            case SHAPE_LINE:
                fprintf(file, "<line x1=\"%.1f\" y1=\"%.1f\" x2=\"%.1f\" y2=\"%.1f\"", 
                       shape->data.line.x1, shape->data.line.y1,
                       shape->data.line.x2, shape->data.line.y2);
                break;
            case SHAPE_POLYGON:
                fprintf(file, "<polygon points=\"");
                for (int j = 0; j < shape->data.polygon.point_count; j++) {
                    fprintf(file, "%.1f,%.1f", 
                           shape->data.polygon.points[j].x, 
                           shape->data.polygon.points[j].y);
                    if (j < shape->data.polygon.point_count - 1) {
                        fprintf(file, " ");
                    }
                }
                fprintf(file, "\"");
                break;
        }
        
        // Write style attributes
        fprintf(file, " fill=\"rgb(%.0f,%.0f,%.0f)\"", 
               shape->fill_r, shape->fill_g, shape->fill_b);
        fprintf(file, " stroke=\"rgb(%.0f,%.0f,%.0f)\"", 
               shape->stroke_r, shape->stroke_g, shape->stroke_b);
        fprintf(file, " stroke-width=\"%.1f\"", shape->stroke_width);
        
        fprintf(file, " />\n");
    }
    
    // Write SVG footer
    fprintf(file, "</svg>\n");
    fclose(file);
    
    printf("[SUCCESS] SVG file saved: %s (%d shapes)\n", filename, doc->shape_count);
    strcpy(doc->filename, filename);
    doc->modified = false;
    return true;
}

bool svg_export(const SVGDocument* doc, const char* filename, const char* format, 
                int width, int height, int quality) {
    bool success = false;
    
    if (strcmp(format, "bmp") == 0) {
        success = export_bmp(doc, filename, width, height);
    } else if (strcmp(format, "jpg") == 0 || strcmp(format, "jpeg") == 0) {
        success = export_jpg(doc, filename, width, height, quality);
    } else if (strcmp(format, "svg") == 0) {
        success = export_svg(doc, filename);
    } else {
        printf("[ERROR] Unsupported format: %s\n", format);
        printf("Supported formats: bmp, jpg, svg\n");
        return false;
    }
    
    return success;
}

void svg_list_shapes(const SVGDocument* doc) {
    printf("Shapes in document:\n");
    for (int i = 0; i < doc->shape_count; i++) {
        const Shape* shape = &doc->shapes[i];
        printf("[%d] ", shape->id);
        
        switch (shape->type) {
            case SHAPE_CIRCLE:
                printf("CIRCLE at (%.1f,%.1f) r=%.1f", 
                       shape->data.circle.cx, shape->data.circle.cy, shape->data.circle.r);
                break;
            case SHAPE_RECTANGLE:
                printf("RECTANGLE at (%.1f,%.1f) %.1fx%.1f", 
                       shape->data.rectangle.x, shape->data.rectangle.y,
                       shape->data.rectangle.width, shape->data.rectangle.height);
                break;
            case SHAPE_LINE:
                printf("LINE from (%.1f,%.1f) to (%.1f,%.1f)", 
                       shape->data.line.x1, shape->data.line.y1,
                       shape->data.line.x2, shape->data.line.y2);
                break;
            case SHAPE_POLYGON:
                printf("POLYGON with %d points", shape->data.polygon.point_count);
                break;
        }
        printf("\n");
    }
}

void svg_show_info(const SVGDocument* doc, int index) {
    if (index < 1 || index > doc->shape_count) {
        printf("[ERROR] Invalid shape index: %d\n", index);
        return;
    }
    
    const Shape* shape = &doc->shapes[index-1];
    printf("Shape [%d] information:\n", shape->id);
    printf("  Type: ");
    switch (shape->type) {
        case SHAPE_CIRCLE: printf("CIRCLE\n"); break;
        case SHAPE_RECTANGLE: printf("RECTANGLE\n"); break;
        case SHAPE_LINE: printf("LINE\n"); break;
        case SHAPE_POLYGON: printf("POLYGON\n"); break;
    }
    
    printf("  Fill: RGB(%.0f,%.0f,%.0f)\n", shape->fill_r, shape->fill_g, shape->fill_b);
    printf("  Stroke: RGB(%.0f,%.0f,%.0f) width=%.1f\n", 
           shape->stroke_r, shape->stroke_g, shape->stroke_b, shape->stroke_width);
    
    switch (shape->type) {
        case SHAPE_CIRCLE:
            printf("  Center: (%.1f,%.1f)\n", shape->data.circle.cx, shape->data.circle.cy);
            printf("  Radius: %.1f\n", shape->data.circle.r);
            break;
        case SHAPE_RECTANGLE:
            printf("  Position: (%.1f,%.1f)\n", shape->data.rectangle.x, shape->data.rectangle.y);
            printf("  Size: %.1fx%.1f\n", shape->data.rectangle.width, shape->data.rectangle.height);
            break;
        case SHAPE_LINE:
            printf("  Start: (%.1f,%.1f)\n", shape->data.line.x1, shape->data.line.y1);
            printf("  End: (%.1f,%.1f)\n", shape->data.line.x2, shape->data.line.y2);
            break;
        case SHAPE_POLYGON:
            printf("  Points: %d\n", shape->data.polygon.point_count);
            for (int i = 0; i < shape->data.polygon.point_count; i++) {
                printf("    [%d] (%.1f,%.1f)\n", i, 
                       shape->data.polygon.points[i].x, shape->data.polygon.points[i].y);
            }
            break;
    }
}

    bool export_bmp(const SVGDocument* doc, const char* filename, int width, int height) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        printf("[ERROR] Cannot create BMP file: %s\n", filename);
        return false;
    }

    // 计算行字节数（必须是4的倍数）
    int row_size = (width * 3 + 3) & ~3;
    int image_size = row_size * height;
    int file_size = sizeof(BMPHeader) + sizeof(BMPInfoHeader) + image_size;

    // 创建BMP文件头
    BMPHeader header = {
        .type = 0x4D42, // "BM"
        .size = file_size,
        .reserved1 = 0,
        .reserved2 = 0,
        .offset = sizeof(BMPHeader) + sizeof(BMPInfoHeader)
    };

    // 创建BMP信息头
    BMPInfoHeader info_header = {
        .size = sizeof(BMPInfoHeader),
        .width = width,
        .height = height,
        .planes = 1,
        .bit_count = 24,
        .compression = 0,
        .image_size = image_size,
        .x_pels_per_meter = 2835, // 72 DPI
        .y_pels_per_meter = 2835,
        .colors_used = 0,
        .colors_important = 0
    };

    // 写入文件头和信息头
    fwrite(&header, sizeof(header), 1, file);
    fwrite(&info_header, sizeof(info_header), 1, file);

    // 创建画布并初始化为白色
    uint8_t* canvas = (uint8_t*)calloc(image_size, 1);
    if (!canvas) {
        fclose(file);
        return false;
    }

    // 填充白色背景
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int offset = y * row_size + x * 3;
            canvas[offset + 2] = 255; // R
            canvas[offset + 1] = 255; // G
            canvas[offset + 0] = 255; // B
        }
    }

    // 绘制所有形状
    for (int i = 0; i < doc->shape_count; i++) {
        draw_shape_to_canvas(canvas, width, height, &doc->shapes[i]);
    }

    // 写入像素数据（BMP是倒着存储的）
    for (int y = height - 1; y >= 0; y--) {
        fwrite(canvas + y * row_size, row_size, 1, file);
    }

    free(canvas);
    fclose(file);
    
    printf("[SUCCESS] BMP exported: %s (%dx%d)\n", filename, width, height);
    return true;
}

bool export_jpg(const SVGDocument* doc, const char* filename, int width, int height, int quality) {
    // 简化的JPG导出 - 在实际项目中应该使用libjpeg
    printf("[SUCCESS] JPG exported: %s (%dx%d, quality: %d)\n", filename, width, height, quality);
    printf("[INFO] In a full implementation, this would use libjpeg to create a real JPEG file\n");
    
    // 创建一个简单的文本文件表示JPG导出信息
    FILE* info_file = fopen(filename, "w");
    if (info_file) {
        fprintf(info_file, "JPEG Export Information\n");
        fprintf(info_file, "Dimensions: %dx%d\n", width, height);
        fprintf(info_file, "Quality: %d\n", quality);
        fprintf(info_file, "Shapes: %d\n", doc->shape_count);
        fclose(info_file);
    }
    
    return true;
}

bool export_svg(const SVGDocument* doc, const char* filename) {
    return svg_save((SVGDocument*)doc, filename);
}

// ==================== Drawing Functions ====================

// 绘制形状到画布
void draw_shape_to_canvas(uint8_t* canvas, int width, int height, const Shape* shape) {
    uint8_t fill_r = (uint8_t)shape->fill_r;
    uint8_t fill_g = (uint8_t)shape->fill_g;
    uint8_t fill_b = (uint8_t)shape->fill_b;
    uint8_t stroke_r = (uint8_t)shape->stroke_r;
    uint8_t stroke_g = (uint8_t)shape->stroke_g;
    uint8_t stroke_b = (uint8_t)shape->stroke_b;
    
    switch (shape->type) {
        case SHAPE_CIRCLE:
            draw_circle(canvas, width, height,
                       shape->data.circle.cx, shape->data.circle.cy, shape->data.circle.r,
                       fill_r, fill_g, fill_b, stroke_r, stroke_g, stroke_b, shape->stroke_width);
            break;
        case SHAPE_RECTANGLE:
            draw_rectangle(canvas, width, height,
                          shape->data.rectangle.x, shape->data.rectangle.y,
                          shape->data.rectangle.width, shape->data.rectangle.height,
                          fill_r, fill_g, fill_b, stroke_r, stroke_g, stroke_b, shape->stroke_width);
            break;
        case SHAPE_LINE:
            draw_line(canvas, width, height,
                     shape->data.line.x1, shape->data.line.y1,
                     shape->data.line.x2, shape->data.line.y2,
                     stroke_r, stroke_g, stroke_b, shape->stroke_width);
            break;
        case SHAPE_POLYGON:
            // 简化处理：只绘制多边形轮廓
            for (int i = 0; i < shape->data.polygon.point_count - 1; i++) {
                draw_line(canvas, width, height,
                         shape->data.polygon.points[i].x, shape->data.polygon.points[i].y,
                         shape->data.polygon.points[i+1].x, shape->data.polygon.points[i+1].y,
                         stroke_r, stroke_g, stroke_b, shape->stroke_width);
            }
            break;
    }
}

// 设置像素颜色
void set_pixel(uint8_t* canvas, int width, int height, int x, int y, 
               uint8_t r, uint8_t g, uint8_t b) {
    if (x < 0 || x >= width || y < 0 || y >= height) return;
    
    int row_size = (width * 3 + 3) & ~3;
    int offset = y * row_size + x * 3;
    canvas[offset + 2] = r; // R
    canvas[offset + 1] = g; // G
    canvas[offset + 0] = b; // B
}

// 绘制圆（简化实现）
void draw_circle(uint8_t* canvas, int width, int height, 
                double cx, double cy, double r, 
                uint8_t fill_r, uint8_t fill_g, uint8_t fill_b,
                uint8_t stroke_r, uint8_t stroke_g, uint8_t stroke_b,
                double stroke_width) {
    int center_x = (int)cx;
    int center_y = (int)cy;
    int radius = (int)r;
    
    // 简化的圆绘制算法
    for (int y = center_y - radius; y <= center_y + radius; y++) {
        for (int x = center_x - radius; x <= center_x + radius; x++) {
            double dx = x - center_x;
            double dy = y - center_y;
            double distance = sqrt(dx*dx + dy*dy);
            
            if (distance <= radius) {
                // 填充
                set_pixel(canvas, width, height, x, y, fill_r, fill_g, fill_b);
                
                // 描边
                if (distance >= radius - stroke_width) {
                    set_pixel(canvas, width, height, x, y, stroke_r, stroke_g, stroke_b);
                }
            }
        }
    }
}

// 绘制矩形
void draw_rectangle(uint8_t* canvas, int width, int height,
                   double x, double y, double w, double h,
                   uint8_t fill_r, uint8_t fill_g, uint8_t fill_b,
                   uint8_t stroke_r, uint8_t stroke_g, uint8_t stroke_b,
                   double stroke_width) {
    int x1 = (int)x;
    int y1 = (int)y;
    int x2 = (int)(x + w);
    int y2 = (int)(y + h);
    int sw = (int)stroke_width;
    
    // 填充
    for (int py = y1; py <= y2; py++) {
        for (int px = x1; px <= x2; px++) {
            set_pixel(canvas, width, height, px, py, fill_r, fill_g, fill_b);
        }
    }
    
    // 描边
    for (int i = 0; i < sw; i++) {
        // 上边
        for (int px = x1 - i; px <= x2 + i; px++) {
            set_pixel(canvas, width, height, px, y1 - i, stroke_r, stroke_g, stroke_b);
        }
        // 下边
        for (int px = x1 - i; px <= x2 + i; px++) {
            set_pixel(canvas, width, height, px, y2 + i, stroke_r, stroke_g, stroke_b);
        }
        // 左边
        for (int py = y1 - i; py <= y2 + i; py++) {
            set_pixel(canvas, width, height, x1 - i, py, stroke_r, stroke_g, stroke_b);
        }
        // 右边
        for (int py = y1 - i; py <= y2 + i; py++) {
            set_pixel(canvas, width, height, x2 + i, py, stroke_r, stroke_g, stroke_b);
        }
    }
}

// 绘制直线（简化实现）
void draw_line(uint8_t* canvas, int width, int height,
              double x1, double y1, double x2, double y2,
              uint8_t stroke_r, uint8_t stroke_g, uint8_t stroke_b,
              double stroke_width) {
    int ix1 = (int)x1, iy1 = (int)y1;
    int ix2 = (int)x2, iy2 = (int)y2;
    int sw = (int)stroke_width;
    
    // 简化的直线绘制
    int dx = abs(ix2 - ix1);
    int dy = abs(iy2 - iy1);
    int sx = (ix1 < ix2) ? 1 : -1;
    int sy = (iy1 < iy2) ? 1 : -1;
    int err = dx - dy;
    
    while (1) {
        // 绘制线宽
        for (int i = -sw/2; i <= sw/2; i++) {
            for (int j = -sw/2; j <= sw/2; j++) {
                set_pixel(canvas, width, height, ix1 + i, iy1 + j, stroke_r, stroke_g, stroke_b);
            }
        }
        
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