#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <stddef.h>

#include "../include/svg_render.h"
#include "../include/image.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

// 创建单位矩阵
Matrix matrix_identity() {
    Matrix I = {{
        {1,0,0},
        {0,1,0},
        {0,0,1}
    }};
    return I;
}
// 矩阵乘法
Matrix matrix_mul(Matrix A, Matrix B) {
    Matrix R = matrix_identity();
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            R.m[i][j] =
                A.m[i][0] * B.m[0][j] +
                A.m[i][1] * B.m[1][j] +
                A.m[i][2] * B.m[2][j];
        }
    }
    return R;
}

// 四种变换
Matrix matrix_translate(float tx, float ty) {
    Matrix M = matrix_identity();
    M.m[0][2] = tx;
    M.m[1][2] = ty;
    return M;
}
Matrix matrix_scale(float sx, float sy) {
    Matrix M = matrix_identity();
    M.m[0][0] = sx;
    M.m[1][1] = sy;
    return M;
}
Matrix matrix_rotate(float angle_deg) {
    float rad = angle_deg * 3.1415926f / 180.0f;
    float c = cos(rad), s = sin(rad);

    Matrix M = matrix_identity();
    M.m[0][0] = c;  M.m[0][1] = -s;
    M.m[1][0] = s;  M.m[1][1] = c;
    return M;
}
Matrix matrix_raw(float a, float b, float c, float d, float e, float f) {
    Matrix M = matrix_identity();
    M.m[0][0] = a;
    M.m[0][1] = c;
    M.m[1][0] = b;
    M.m[1][1] = d;
    M.m[0][2] = e;
    M.m[1][2] = f;
    return M;
}
// 矩阵应用到点
void matrix_apply_point(Matrix *M, float *x, float *y) {
    float X = (*x), Y = (*y);

    float new_x = M->m[0][0] * X + M->m[0][1] * Y + M->m[0][2];
    float new_y = M->m[1][0] * X + M->m[1][1] * Y + M->m[1][2];

    *x = new_x;
    *y = new_y;
}

// 对group创建矩阵（递归）
Matrix compute_group_matrix(SVGGroup *g) {
    if (!g) return matrix_identity();

    // 先取 parent 的矩阵
    Matrix parentM = compute_group_matrix(g->parent);

    // 自己的 transform 矩阵
    Matrix self = matrix_identity();

    for (int i = 0; i < g->transform_count; i++) {
        Transform *t = &g->transforms[i];

        Matrix M2 = matrix_identity();
        switch (t->type) {
            case TRANSFORM_TRANSLATE:
                M2 = matrix_translate(t->x, t->y);
                break;
            case TRANSFORM_SCALE:
                M2 = matrix_scale(t->x, t->y);
                break;
            case TRANSFORM_ROTATE:
                M2 = matrix_rotate(t->angle);
                break;
            case TRANSFORM_MATRIX:
                M2 = matrix_raw(t->matrix.a, t->matrix.b, t->matrix.c, t->matrix.d, t->matrix.e, t->matrix.f);
                break;
        }

        // 右乘累计
        self = matrix_mul(self, M2);
    }

    return matrix_mul(parentM, self);
}

Matrix compute_shape_matrix(SVGShape *shape) {
    Matrix M = compute_group_matrix(shape->group);

    for (int i = 0; i < shape->transform_count; i++) {
        Transform *t = &shape->transforms[i];

        Matrix M2 = matrix_identity();
        switch (t->type) {
            case TRANSFORM_TRANSLATE:
                M2 = matrix_translate(t->x, t->y);
                break;
            case TRANSFORM_SCALE:
                M2 = matrix_scale(t->x, t->y);
                break;
            case TRANSFORM_ROTATE:
                M2 = matrix_rotate(t->angle);
                break;
            case TRANSFORM_MATRIX:
                M2 = matrix_raw(t->matrix.a, t->matrix.b, t->matrix.c, t->matrix.d, t->matrix.e, t->matrix.f);
                break;
        }

        // shape 的转换也往右乘
        M = matrix_mul(M, M2);
    }

    return M;
}

void apply_all_transforms(SVGShape *shape, float *x, float *y) {
    Matrix M = compute_shape_matrix(shape);
    matrix_apply_point(&M, x, y);
}




// 解析变换字符串
int parse_transform(const char *transform_str, Transform **transforms, int *transform_count) {
    if (!transform_str || strlen(transform_str) == 0) {
        return 0;
    }
    
    char *str = strdup(transform_str);
    char *saveptr;
    const char *token = strtok_r(str, ")", &saveptr);
    
    int capacity = 5;
    *transforms = malloc(capacity * sizeof(Transform));
    *transform_count = 0;
    
    
    while (token) {
        // 去除空白
        skip_spaces(&token);
        
        if (strncmp(token, "translate(", 10) == 0) {
           
            float x = 0, y = 0;
            sscanf(token + 10, "%f,%f", &x, &y);
            
            if (*transform_count >= capacity) {
                capacity *= 2;
                *transforms = realloc(*transforms, capacity * sizeof(Transform));
            }
            
            Transform *t = &(*transforms)[(*transform_count)++];
            t->type = TRANSFORM_TRANSLATE;
            t->x = x;
            t->y = y;
        }
        else if (strncmp(token, "rotate(", 7) == 0) {
            
            float angle = 0, cx = 0, cy = 0;
            int params = sscanf(token + 7, "%f,%f,%f", &angle, &cx, &cy);
            
            if (*transform_count >= capacity) {
                capacity *= 2;
                *transforms = realloc(*transforms, capacity * sizeof(Transform));
            }
            
            Transform *t = &(*transforms)[(*transform_count)++];
            t->type = TRANSFORM_ROTATE;
            t->angle = angle * M_PI / 180.0; // 转换为弧度
            if (params >= 3) {
                t->cx = cx;
                t->cy = cy;
            }
        }
        else if (strncmp(token, "scale(", 6) == 0) {
            
            float x = 1, y = 1;
            sscanf(token + 6, "%f,%f", &x, &y);
            
            if (*transform_count >= capacity) {
                capacity *= 2;
                *transforms = realloc(*transforms, capacity * sizeof(Transform));
            }
            
            Transform *t = &(*transforms)[(*transform_count)++];
            t->type = TRANSFORM_SCALE;
            t->x = x;
            t->y = y;
        }
        else if (strncmp(token, "matrix(", 7) == 0) {
            TransformMatrix m;
            sscanf(token + 7, "%f,%f,%f,%f,%f,%f", &m.a, &m.b, &m.c, &m.d, &m.e, &m.f);
            
            if (*transform_count >= capacity) {
                capacity *= 2;
                *transforms = realloc(*transforms, capacity * sizeof(Transform));
            }
            
            Transform *t = &(*transforms)[(*transform_count)++];
            t->type = TRANSFORM_MATRIX;
            t->matrix = m;
        }
        
        token = strtok_r(NULL, ")", &saveptr);
    }
    
    free(str);
    return *transform_count > 0;
}

// 应用变换到点
void apply_transforms_to_point(Transform *transforms, int count, float *x, float *y) {
    for (int i = 0; i < count; i++) {
        Transform *t = &transforms[i];
        float tx = *x, ty = *y;
        
        switch (t->type) {
            case TRANSFORM_TRANSLATE:
                *x += t->x;
                *y += t->y;
                break;
                
            case TRANSFORM_ROTATE:
                // 如果有旋转中心，先平移到原点
                if (t->cx != 0 || t->cy != 0) {
                    tx -= t->cx;
                    ty -= t->cy;
                }
                
                // 旋转
                {
                    float cos_a = cos(t->angle);
                    float sin_a = sin(t->angle);
                    float new_x = tx * cos_a - ty * sin_a;
                    float new_y = tx * sin_a + ty * cos_a;
                    tx = new_x;
                    ty = new_y;
                }
                
                // 如果有旋转中心，平移回去
                if (t->cx != 0 || t->cy != 0) {
                    tx += t->cx;
                    ty += t->cy;
                }
                
                *x = tx;
                *y = ty;
                break;
                
            case TRANSFORM_SCALE:
                *x *= t->x;
                *y *= t->y;
                break;
                
            case TRANSFORM_MATRIX:
                {
                    float new_x = t->matrix.a * tx + t->matrix.c * ty + t->matrix.e;
                    float new_y = t->matrix.b * tx + t->matrix.d * ty + t->matrix.f;
                    *x = new_x;
                    *y = new_y;
                }
                break;
                
            default:
                break;
        }
    }
}



// 解析十六进制颜色和 rgb() 颜色
RGBColor parse_color(const char *color_str) {
    RGBColor color = {0, 0, 0};
    if (color_str[0] == '#') {
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
        
    } else if (strstr(color_str, "rgb(")) {
        int r, g, b;
        sscanf(color_str, "rgb(%d,%d,%d)", &r, &g, &b);
        color.r = r;
        color.g = g;
        color.b = b;
    
    } else {
        struct ColorMap {
            const char *name;
            RGBColor color;
        };
        struct ColorMap color_map[] = {
            {"red", {255, 0, 0}},
            {"green", {0, 255, 0}},
            {"blue", {0, 0, 255}},
            {"black", {0, 0, 0}},
            {"white", {255, 255, 255}},
            {"yellow", {255, 255, 0}},
            {"cyan", {0, 255, 255}},
            {"magenta", {255, 0, 255}},
            {"gray", {128, 128, 128}},
            {"grey", {128, 128, 128}},
            {"orange", {255, 165, 0}},
            {"purple", {128, 0, 128}},
            {"brown", {165, 42, 42}},
            {"pink", {255, 192, 203}},
            {"none", {255, 255, 255}}
        };
       
        for (int i = 0; i < sizeof(color_map)/sizeof(color_map[0]); i++) {
            if (strcmp(color_str, color_map[i].name) == 0) {
                return color_map[i].color;
            }
        }
    }
    return color;
}

// 从字符串中提取属性值
char* extract_attribute(const char *line, const char *attr_name) {
    char search_str[100];
    snprintf(search_str, sizeof(search_str), "%s=\"", attr_name);
    
    char *start = strstr(line, search_str);
    if (!start) {
        return NULL;
    }
    
    start += strlen(search_str);
    char *end = strchr(start, '"');
    if (!end) return NULL;
    
    int len = end - start;
    char *result = malloc(len + 1);
    strncpy(result, start, len);
    result[len] = '\0';
    
    return result;
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

    // 分组栈
    SVGGroup *group_stack[100];
    int group_stack_size = 0;
    SVGGroup *current_group = NULL;
    
    while (fgets(line, sizeof(line), file)) {
        // 去掉前后空白
        const char *p = line;
        skip_spaces(&p);

        char tag[4096];
        read_full_tag(file, line, tag);

        char *trimmed = tag;
        while (*trimmed == ' ' || *trimmed == '\t') trimmed++;


        // 检查分组开始 <g>
        if (strstr(trimmed, "<g")) {
            SVGGroup *new_group = malloc(sizeof(SVGGroup));
            new_group->transforms = NULL;
            new_group->transform_count = 0;
            new_group->parent = current_group;
            
            // 解析分组变换
            char *transform_val = extract_attribute(trimmed, "transform");
            if (transform_val) {
                parse_transform(transform_val, &new_group->transforms, &new_group->transform_count);
                printf("解析分组变换: %d 个变换\n", new_group->transform_count);
                free(transform_val);
            }
            
            // 压入分组栈
            if (group_stack_size < 100) {
                group_stack[group_stack_size++] = new_group;
                current_group = new_group;
            }
            
            continue;
        }
        
        // 检查分组结束 </g>
        if (strstr(trimmed, "</g>")) {
            if (group_stack_size > 0) {
                // 弹出分组栈
                group_stack_size--;
                if (group_stack_size > 0) {
                    current_group = group_stack[group_stack_size - 1];
                } else {
                    current_group = NULL;
                }
            }
            continue;
        }
        
        if (strstr(trimmed, "<rect")) {
            if (*shape_count >= capacity) {
                capacity *= 2;
                *shapes = realloc(*shapes, capacity * sizeof(SVGShape));
            }
            
            SVGShape *shape = &(*shapes)[(*shape_count)++];
            strcpy(shape->type, "rect");
            shape->transforms = NULL;
            shape->transform_count = 0;
            shape->group = current_group;
            
            char *x_val = extract_attribute(trimmed, "x");
            char *y_val = extract_attribute(trimmed, "y");
            char *width_val = extract_attribute(trimmed, "width");
            char *height_val = extract_attribute(trimmed, "height");
            char *fill_val = extract_attribute(trimmed, "fill");
            char *transform_val = extract_attribute(trimmed, "transform");
            
            
            shape->x = x_val ? atof(x_val) : 0;
            shape->y = y_val ? atof(y_val) : 0;
            shape->width = width_val ? atof(width_val) : 100;
            shape->height = height_val ? atof(height_val) : 100;
            
            if (fill_val) {
                shape->color = parse_color(fill_val);
            } else {
                shape->color.r = 0;
                shape->color.g = 0;
                shape->color.b = 255;
            }
            
            // 解析变换
            if (transform_val) {
                parse_transform(transform_val, &shape->transforms, &shape->transform_count);
            }
            
            // 释放内存
            free(x_val);
            free(y_val);
            free(width_val);
            free(height_val);
            free(fill_val);
            free(transform_val);
            
            }
        
        else if (strstr(trimmed, "<circle")) {
            if (*shape_count >= capacity) {
                capacity *= 2;
                *shapes = realloc(*shapes, capacity * sizeof(SVGShape));
            }
            
            SVGShape *shape = &(*shapes)[(*shape_count)++];
            strcpy(shape->type, "circle");
            shape->transforms = NULL;
            shape->transform_count = 0;
            shape->group = current_group;
            
            char *cx_val = extract_attribute(trimmed, "cx");
            char *cy_val = extract_attribute(trimmed, "cy");
            char *r_val = extract_attribute(trimmed, "r");
            char *fill_val = extract_attribute(trimmed, "fill");
            char *transform_val = extract_attribute(trimmed, "transform");
            
            shape->cx = cx_val ? atof(cx_val) : 100;
            shape->cy = cy_val ? atof(cy_val) : 100;
            shape->r = r_val ? atof(r_val) : 50;
            
            if (fill_val) {
                shape->color = parse_color(fill_val);
            } else {
                shape->color.r = 255;
                shape->color.g = 0;
                shape->color.b = 0;
            }
            
            // 解析变换
            if (transform_val) {
                parse_transform(transform_val, &shape->transforms, &shape->transform_count);
            }
            
            // 释放内存
            free(cx_val);
            free(cy_val);
            free(r_val);
            free(fill_val);
            free(transform_val);
        }
        else if (strstr(trimmed, "<line")) {
            if (*shape_count >= capacity) {
                capacity *= 2;
                *shapes = realloc(*shapes, capacity * sizeof(SVGShape));
            }
            
            SVGShape *shape = &(*shapes)[(*shape_count)++];
            strcpy(shape->type, "line");
            shape->transforms = NULL;
            shape->transform_count = 0;
            shape->group = current_group;
            
            // 解析属性
            char *x1_val = extract_attribute(trimmed, "x1");
            char *y1_val = extract_attribute(trimmed, "y1");
            char *x2_val = extract_attribute(trimmed, "x2");
            char *y2_val = extract_attribute(trimmed, "y2");
            char *stroke_val = extract_attribute(trimmed, "stroke");
            char *transform_val = extract_attribute(trimmed, "transform");
            
            shape->x = x1_val ? atof(x1_val) : 0;
            shape->y = y1_val ? atof(y1_val) : 0;
            shape->x2 = x2_val ? atof(x2_val) : 100;
            shape->y2 = y2_val ? atof(y2_val) : 100;
            
            if (stroke_val) {
                shape->color = parse_color(stroke_val);
            } else {
                shape->color.r = 0;
                shape->color.g = 255;
                shape->color.b = 0;
            }
            
            // 解析变换
            if (transform_val) {
                parse_transform(transform_val, &shape->transforms, &shape->transform_count);
            }
            
            // 释放内存
            free(x1_val);
            free(y1_val);
            free(x2_val);
            free(y2_val);
            free(stroke_val);
            free(transform_val);
        }
    }
    
    fclose(file);
    return 1;
}

// 绘制线条
void draw_line(Image *img, float x1, float y1, float x2, float y2, RGBColor color) {
    int ix1 = (int)x1, iy1 = (int)y1;
    int ix2 = (int)x2, iy2 = (int)y2;
    
    // Bresenham's line algorithm
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


// 绘制矩形
void draw_rectangle(Image *img, float x, float y, float width, float height, RGBColor color) {
    // 对于无旋转的矩形，使用简单填充
    int ix = (int)x, iy = (int)y;
    int iw = (int)width, ih = (int)height;
    
    // 确保坐标在图像范围内
    if (ix < 0) ix = 0;
    if (iy < 0) iy = 0;
    if (ix + iw > img->width) iw = img->width - ix;
    if (iy + ih > img->height) ih = img->height - iy;
    
    // 填充矩形
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

    // 确保半径在合理范围内
    if (ir <= 0) return;
    
    for (int y = -ir; y <= ir; y++) {
        for (int x = -ir; x <= ir; x++) {
            if (x*x + y*y <= ir*ir) {
                int px = icx + x;
                int py = icy + y;
                if (px >= 0 && px < img->width && py >= 0 && py < img->height) {
                    set_pixel(img, px, py, color);
                }
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
        
        //打印调试信息
        printf("绘制形状 %d: 类型=%s, 颜色=(%d, %d, %d), 变换数量=%d\n", i, shape->type, shape->color.r, shape->color.g, shape->color.b, shape->transform_count);
        
        if (strcmp(shape->type, "rect") == 0) {
            float x = shape->x, y = shape->y, width = shape->width, height = shape->height;
            
            // 应用变换到矩形的四个角
            if (shape->group || (shape->transforms && shape->transform_count > 0)) {
                
                // 变换矩形的四个角点
                float x1 = x, y1 = y;
                float x2 = x + width, y2 = y;
                float x3 = x + width, y3 = y + height;
                float x4 = x, y4 = y + height;
                
                apply_all_transforms(shape, &x1, &y1);
                apply_all_transforms(shape, &x2, &y2);
                apply_all_transforms(shape, &x3, &y3);
                apply_all_transforms(shape, &x4, &y4);
                
                // 计算变换后的边界框
                float min_x = fmin(fmin(x1, x2), fmin(x3, x4));
                float min_y = fmin(fmin(y1, y2), fmin(y3, y4));
                float max_x = fmax(fmax(x1, x2), fmax(x3, x4));
                float max_y = fmax(fmax(y1, y2), fmax(y3, y4));
                
                x = min_x;
                y = min_y;
                width = max_x - min_x;
                height = max_y - min_y;
                
                printf("  变换后矩形: (%.1f,%.1f) w=%.1f h=%.1f\n", x, y, width, height);
            }
            
            draw_rectangle(img, x, y, width, height, shape->color);
        } else if (strcmp(shape->type, "circle") == 0) {
            float cx = shape->cx, cy = shape->cy, r = shape->r;
            
            // 应用变换
            if (shape->group || (shape->transforms && shape->transform_count > 0)) {
                float temp_cx = cx, temp_cy = cy;
                apply_all_transforms(shape, &temp_cx, &temp_cy);
                cx = temp_cx;
                cy = temp_cy;
                
                // 对于缩放变换，调整半径
                for (int j = 0; j < shape->transform_count; j++) {
                    Transform *t = &shape->transforms[j];
                    if (t->type == TRANSFORM_SCALE) {
                        r *= (t->x + t->y) / 2.0; // 使用平均缩放因子
                    }
                }
            }
            
            draw_circle(img, cx, cy, r, shape->color);
        } else if (strcmp(shape->type, "line") == 0) {
            float x1 = shape->x, y1 = shape->y, x2 = shape->x2, y2 = shape->y2;
            
            // 应用变换
            if (shape->group || (shape->transforms && shape->transform_count > 0)) {
                float temp_x1 = x1, temp_y1 = y1;
                float temp_x2 = x2, temp_y2 = y2;
                
                apply_all_transforms(shape, &temp_x1, &temp_y1);
                apply_all_transforms(shape, &temp_x2, &temp_y2);
                
                x1 = temp_x1;
                y1 = temp_y1;
                x2 = temp_x2;
                y2 = temp_y2;
            }
            
            draw_line(img, x1, y1, x2, y2, shape->color);
        }
    }
}

