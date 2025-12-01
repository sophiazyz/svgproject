#ifndef SVG_GUI_TYPES_H
#define SVG_GUI_TYPES_H

#define MAX_SHAPES 100

typedef enum {
    SVG_SHAPE_CIRCLE,
    SVG_SHAPE_RECT,
    SVG_SHAPE_LINE
} SvgShapeType;

typedef struct {
    double cx, cy, r;
    char fill[16];
} SvgCircle;

typedef struct {
    double x, y, width, height;
    char fill[16];
} SvgRect;

typedef struct {
    double x1, y1, x2, y2;
    char stroke[16];
} SvgLine;

typedef struct {
    SvgShapeType type;
    union {
        SvgCircle circle;
        SvgRect rect;
        SvgLine line;
    } data;
    int id;
} SvgShape;

typedef struct {
    double width, height;
    SvgShape shapes[MAX_SHAPES];
    int shape_count;
} SvgDocument;

// 函数声明
SvgDocument* create_svg_document(double width, double height);
int export_to_bmp(SvgDocument* doc, const char* filename);
int export_to_jpg(SvgDocument* doc, const char* filename);

#endif