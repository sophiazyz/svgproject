#ifndef SVG_CORE_H
#define SVG_CORE_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_SHAPES 100
#define MAX_COMMAND_LENGTH 256
#define MAX_FILENAME_LENGTH 128

typedef enum {
    SHAPE_CIRCLE,
    SHAPE_RECTANGLE,
    SHAPE_LINE,
    SHAPE_POLYGON
} ShapeType;

typedef struct {
    double x, y;
} Point;

typedef struct {
    ShapeType type;
    int id;
    
    // Common attributes
    double fill_r, fill_g, fill_b;
    double stroke_r, stroke_g, stroke_b;
    double stroke_width;
    
    // Shape-specific data
    union {
        struct { double cx, cy, r; } circle;
        struct { double x, y, width, height; } rectangle;
        struct { double x1, y1, x2, y2; } line;
        struct { Point* points; int point_count; } polygon;
    } data;
    
    bool selected;
} Shape;

typedef struct {
    Shape shapes[MAX_SHAPES];
    int shape_count;
    int selected_shape;
    char filename[MAX_FILENAME_LENGTH];
    bool modified;
    double zoom;
    double pan_x, pan_y;
} SVGDocument;

// Core functions
void svg_init(SVGDocument* doc);
void svg_cleanup(SVGDocument* doc);
bool svg_load(SVGDocument* doc, const char* filename);
bool svg_save(SVGDocument* doc, const char* filename);
bool svg_export(const SVGDocument* doc, const char* filename, const char* format, 
                int width, int height, int quality);
void svg_list_shapes(const SVGDocument* doc);
void svg_show_info(const SVGDocument* doc, int index);

// Export functions
bool export_bmp(const SVGDocument* doc, const char* filename, int width, int height);
bool export_jpg(const SVGDocument* doc, const char* filename, int width, int height, int quality);
bool export_png(const SVGDocument* doc, const char* filename, int width, int height);
bool export_svg(const SVGDocument* doc, const char* filename);

// Drawing functions (for export)
void draw_shape_to_canvas(uint8_t* canvas, int width, int height, const Shape* shape);
void set_pixel(uint8_t* canvas, int width, int height, int x, int y, 
               uint8_t r, uint8_t g, uint8_t b);
void draw_circle(uint8_t* canvas, int width, int height, 
                double cx, double cy, double r, 
                uint8_t fill_r, uint8_t fill_g, uint8_t fill_b,
                uint8_t stroke_r, uint8_t stroke_g, uint8_t stroke_b,
                double stroke_width);
void draw_rectangle(uint8_t* canvas, int width, int height,
                   double x, double y, double w, double h,
                   uint8_t fill_r, uint8_t fill_g, uint8_t fill_b,
                   uint8_t stroke_r, uint8_t stroke_g, uint8_t stroke_b,
                   double stroke_width);
void draw_line(uint8_t* canvas, int width, int height,
              double x1, double y1, double x2, double y2,
              uint8_t stroke_r, uint8_t stroke_g, uint8_t stroke_b,
              double stroke_width);

#endif