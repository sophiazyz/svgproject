#ifndef SVG_EDITOR_H
#define SVG_EDITOR_H

#include "svg_render.h"

typedef struct {
    SVGShape *shapes;
    int shape_count;
    int selected_shape;
    char filename[256];
    int modified;
    float zoom;
    float pan_x, pan_y;
} SVGEditor;

// 编辑器初始化和管理
SVGEditor* editor_create();
void editor_free(SVGEditor *editor);
int editor_load(SVGEditor *editor, const char *filename);
int editor_save(SVGEditor *editor, const char *filename);
void editor_clear(SVGEditor *editor);

// 形状操作
int editor_add_circle(SVGEditor *editor, float cx, float cy, float r, RGBColor fill);
int editor_add_rectangle(SVGEditor *editor, float x, float y, float w, float h, RGBColor fill);
int editor_add_line(SVGEditor *editor, float x1, float y1, float x2, float y2, RGBColor stroke);
int editor_select_shape(SVGEditor *editor, int index);
void editor_deselect(SVGEditor *editor);
int editor_delete_selected(SVGEditor *editor);
int editor_move_shape(SVGEditor *editor, int index, float dx, float dy);

// 属性修改
int editor_set_fill_color(SVGEditor *editor, int index, RGBColor color);
int editor_set_stroke_color(SVGEditor *editor, int index, RGBColor color);

// 视图控制
void editor_set_zoom(SVGEditor *editor, float zoom);
void editor_pan(SVGEditor *editor, float dx, float dy);
void editor_reset_view(SVGEditor *editor);
void editor_fit_to_content(SVGEditor *editor);

// 导出功能
int editor_export_image(SVGEditor *editor, const char *filename, const char *format);

// 信息显示
void editor_list_shapes(SVGEditor *editor);
void editor_show_shape_info(SVGEditor *editor, int index);
void editor_show_status(SVGEditor *editor);

#endif