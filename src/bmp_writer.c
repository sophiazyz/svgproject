#include "../include/bmp_writer.h"
#include <stdio.h>

void write_bmp(const char *filename, Image *img) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "fail to open file: %s\n", filename);
        return;
    }
    
    int width = img->width;
    int height = img->height;
    int padding = (4 - (width * 3) % 4) % 4;
    int image_size = (width * 3 + padding) * height;
    int file_size = 54 + image_size;
    
    // BMP文件头
    unsigned char file_header[14] = {
        'B', 'M',           // 签名
        file_size, file_size >> 8, file_size >> 16, file_size >> 24, // 文件大小
        0, 0, 0, 0,         // 保留
        54, 0, 0, 0         // 像素数据偏移
    };
    
    // BMP信息头
    unsigned char info_header[40] = {
        40, 0, 0, 0,        // 信息头大小
        width, width >> 8, width >> 16, width >> 24,      // 宽度
        height, height >> 8, height >> 16, height >> 24,  // 高度
        1, 0,               // 颜色平面数
        24, 0,              // 每像素位数
        0, 0, 0, 0,         // 压缩方式
        image_size, image_size >> 8, image_size >> 16, image_size >> 24, // 图像数据大小
        0, 0, 0, 0,         // 水平分辨率
        0, 0, 0, 0,         // 垂直分辨率
        0, 0, 0, 0,         // 调色板颜色数
        0, 0, 0, 0          // 重要颜色数
    };
    
    fwrite(file_header, 1, 14, file);
    fwrite(info_header, 1, 40, file);
    
    // 像素数据 (BGR格式，从下到上)
    unsigned char padding_data[3] = {0, 0, 0};
    for (int y = height - 1; y >= 0; y--) {
        for (int x = 0; x < width; x++) {
            RGBColor pixel = img->pixels[y * width + x];
            unsigned char color[3] = {pixel.b, pixel.g, pixel.r};
            fwrite(color, 1, 3, file);
        }
        fwrite(padding_data, 1, padding, file);
    }
    
    fclose(file);
}