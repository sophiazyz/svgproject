#ifndef SVG_RENDER_H
#define SVG_RENDER_H

#include <stdint.h>

// ----------------------------
// 变换结构体
// ----------------------------
typedef struct Transform {
    double rotate_angle;      // 旋转角度（度）
    double translate_x;       // 平移 X
    double translate_y;       // 平移 Y
    struct Transform *next;   // 链表，支持多个变换叠加
    Transform Type type;
    float x, y;        // 用于平移、缩放
    float angle;       // 用于旋转
    float cx, cy;      // 旋转中心
    Transform Matrix matrix; // 矩阵变换
} Transform;


// ----------------------------
// 位图画布结构体
// ----------------------------
typedef struct {
    int width;
    int height;
    unsigned char *pixels; // RGB 数据，每个像素 3 个字节
} BitmapCanvas;

// ----------------------------
// SVG 基本形状类型
// ----------------------------
typedef enum {
    SVG_SHAPE_CIRCLE,
    SVG_SHAPE_RECT,
    SVG_SHAPE_ROUNDED_RECT,
    SVG_SHAPE_LINE
} SvgShapeType;

// 圆形
typedef struct {
    double cx, cy;
    double r;
    uint32_t fill_color;
    uint32_t stroke_color;
    double stroke_width;
    double opacity;
    Transform *transform;
} SvgCircle;

// 矩形
typedef struct {
    double x, y;
    double width, height;
    uint32_t fill_color;
    uint32_t stroke_color;
    double stroke_width;
    double opacity;
    Transform *transform;
} SvgRect;

// 圆角矩形
typedef struct {
    double x, y;
    double width, height;
    double rx, ry;           // 圆角半径
    uint32_t fill_color;
    uint32_t stroke_color;
    double stroke_width;
    double opacity;
    Transform *transform;
} SvgRoundedRect;

// 线
typedef struct {
    double x1, y1;
    double x2, y2;
    uint32_t stroke_color;
    double stroke_width;
    double opacity;
    Transform *transform;
} SvgLine;

// SVGShape 联合体
typedef struct SvgShape {
    SvgShapeType type;
    union {
        SvgCircle circle;
        SvgRect rect;
        SvgRoundedRect rounded_rect;
        SvgLine line;
    } data;
    int id;
    struct SvgShape *next;
} SvgShape;

// SVG 文档
typedef struct {
    double width;
    double height;
    SvgShape *shapes;
} SvgDocument;

// ----------------------------
// 函数声明
// ----------------------------

// 创建和释放画布
BitmapCanvas* create_canvas(int width, int height);
void free_canvas(BitmapCanvas *canvas);

// 将 SvgDocument 渲染到 BitmapCanvas
void render_svg_to_bitmap(SvgDocument *doc, BitmapCanvas **canvas_out);

#endif // SVG_RENDER_H
