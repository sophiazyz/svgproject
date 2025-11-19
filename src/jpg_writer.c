#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <jpeglib.h>

#include "../include/jpg_writer.h"

void write_jpg(const char *filename, Image *img, int quality) {
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    
    FILE *outfile = fopen(filename, "wb");
    if (!outfile) return;
    
    // 设置 JPEG 压缩参数
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, outfile);
    
    // 图像信息
    cinfo.image_width = img->width;
    cinfo.image_height = img->height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    
    // 开始压缩
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);
    jpeg_start_compress(&cinfo, TRUE);
    
    // 写入像素数据
    JSAMPROW row_pointer[1];
    int row_stride = img->width * 3;
    unsigned char *row_buffer = malloc(row_stride);
    
    // 逐行写入图像数据(RGB 格式)
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            RGBColor pixel = img->pixels[y * img->width + x];
            row_buffer[x * 3] = pixel.r;
            row_buffer[x * 3 + 1] = pixel.g;
            row_buffer[x * 3 + 2] = pixel.b;
        }
        row_pointer[0] = row_buffer;
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }
    
    free(row_buffer);
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(outfile);
}