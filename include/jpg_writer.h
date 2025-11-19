#ifndef JPG_WRITER_H
#define JPG_WRITER_H

#include "image.h"

void write_jpg(const char *filename, Image *img, int quality);

#endif