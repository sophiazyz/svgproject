#ifndef SVG_RENDER_H
#define SVG_RENDER_H

#include "image.h"

#include <stdint.h>

// Color conversion utilities
uint32_t hex_color_to_rgb(unsigned int hex_color);
void rgb_to_components(uint32_t rgb, uint8_t *r, uint8_t *g, uint8_t *b);


// 变换矩阵结构
typedef struct {
    float a, b, c, d, e, f;
} TransformMatrix;

// 变换类型枚举
typedef enum {
    TRANSFORM_NONE,
    TRANSFORM_TRANSLATE,
    TRANSFORM_ROTATE,
    TRANSFORM_SCALE,
    TRANSFORM_MATRIX
} TransformType;

// 变换结构
typedef struct {
    TransformType type;
    float x, y;        // 用于平移、缩放
    float angle;       // 用于旋转
    float cx, cy;      // 旋转中心
    TransformMatrix matrix; // 矩阵变换
} Transform;

// 3×3 矩阵
typedef struct {
    float m[3][3];
} Matrix;

// 分组结构
typedef struct SVGGroup {
    Transform *transforms;
    int transform_count;
    
    struct SVGGroup *parent;
} SVGGroup;

typedef struct {
    char type[20];
    float x, y, x2, y2, cx, cy, r, width, height;
    RGBColor color;
    Transform *transforms;
    int transform_count;
    SVGGroup *group;  // 所属分组
} SVGShape;


int parse_svg(const char *filename, SVGShape **shapes, int *shape_count);

Matrix matrix_identity();
Matrix matrix_mul(Matrix A, Matrix B);
Matrix matrix_translate(float tx, float ty);
Matrix matrix_scale(float sx, float sy);
Matrix matrix_rotate(float angle_deg);
Matrix matrix_raw(float a, float b, float c, float d, float e, float f);
void matrix_apply_point(Matrix *M, float *x, float *y);

Matrix compute_group_matrix(SVGGroup *g);
Matrix compute_shape_matrix(SVGShape *shape);

void draw_line(Image *img, float x1, float y1, float x2, float y2, RGBColor color);
void draw_rectangle(Image *img, float x, float y, float width, float height, RGBColor color);
void draw_circle(Image *img, float cx, float cy, float r, RGBColor color);
void render_svg_to_image(Image *img, SVGShape *shapes, int shape_count);
void apply_transforms(SVGShape *shape, float *x, float *y);
void apply_transforms_to_point(Transform *transforms, int count, float *x, float *y);
void apply_group_transforms(SVGGroup *group, float *x, float *y);
int parse_transform(const char *transform_str, Transform **transforms, int *transform_count);

#endif