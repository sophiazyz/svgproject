#ifndef GUI_RENDER_H
#define GUI_RENDER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "svg_types.h"
#include "svg_parser.h"


typedef struct {
    SDL_Window* window; 
    SDL_Renderer* renderer;
    TTF_Font* font;
    SvgDocument* document;
    SvgShape* selected_shape;
    int zoom_level;
    double pan_x, pan_y;
    int is_dragging;
    int drag_start_x, drag_start_y;
    double selected_original_x, selected_original_y;
} GuiContext;


void render_shape(SDL_Renderer* renderer, SvgShape* shape, double pan_x, double pan_y, double scale);
void render_selection_handles(SDL_Renderer* renderer, SvgShape* shape, double pan_x, double pan_y, double scale);
void render_toolbar(SDL_Renderer* renderer, TTF_Font* font, SvgShape* selected_shape);
GuiContext* gui_create();
void gui_destroy(GuiContext* ctx);

int parse_svg_file(const char* filename, SvgDocument* doc);
void display_summary(SvgDocument* doc);
void display_shapes(SvgDocument* doc);
void add_circle(SvgDocument* doc, double cx, double cy, double r, const char* fill);
void add_rect(SvgDocument* doc, double x, double y, double w, double h, const char* fill);
void select_shape(SvgDocument* doc, int id);
void move_shape(SvgDocument* doc, double dx, double dy);
void delete_shape(SvgDocument* doc);
void gui_parse_color(const char* color_str, char* output);

#endif // GUI_RENDER_H