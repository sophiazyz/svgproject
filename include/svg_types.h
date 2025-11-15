#ifndef SVG_TYPES_H
#define SVG_TYPES_H

typedef enum
{
    SVG_SHAPE_CIRCLE,
    SVG_SHAPE_RECT,
    SVG_SHAPE_LINE
} SvgShapeType;

typedef struct
{
    double cx, cy, r;
    unsigned int fill_color; // 0xRRGGBB format
} SvgCircle;

typedef struct
{
    double x, y, width, height;
    unsigned int fill_color; // 0xRRGGBB format
} SvgRect;

typedef struct
{
    double x1, y1, x2, y2;
    unsigned int stroke_color; // 0xRRGGBB format
} SvgLine;

typedef struct SvgShape
{
    SvgShapeType type;
    union
    {
        SvgCircle circle;
        SvgRect rect;
        SvgLine line;
    } data;
    int id;
    struct SvgShape *next; // for linked list
} SvgShape;

typedef struct
{
    double width, height; // SVG canvas size
    SvgShape *shapes;     // head of shape list
} SvgDocument;

#endif