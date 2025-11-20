#include "../include/gui_render.h"

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800
#define TOOLBAR_WIDTH 200
#define CANVAS_COLOR 0xF0F0F0

void render_shape(SDL_Renderer* renderer, SvgShape* shape, double pan_x, double pan_y, double scale) {
    if (!shape) return;
    
    switch (shape->type) {
        case SVG_SHAPE_CIRCLE: {
            SvgCircle* circle = &shape->data.circle;
            int cx = (int)((circle->cx + pan_x) * scale);
            int cy = (int)((circle->cy + pan_y) * scale);
            int r = (int)(circle->r * scale);
            
            // Convert color
            int r_color = (circle->fill_color >> 16) & 0xFF;
            int g_color = (circle->fill_color >> 8) & 0xFF;
            int b_color = circle->fill_color & 0xFF;
            
            SDL_SetRenderDrawColor(renderer, r_color, g_color, b_color, 255);
            
            // Draw filled circle (simplified - for real implementation use circle algorithm)
            for (int angle = 0; angle < 360; angle++) {
                double rad = angle * M_PI / 180.0;
                int x = cx + (int)(r * cos(rad));
                int y = cy + (int)(r * sin(rad));
                SDL_RenderDrawPoint(renderer, x, y);
            }
            break;
        }
        
        case SVG_SHAPE_RECT: {
            SvgRect* rect = &shape->data.rect;
            SDL_Rect sdl_rect = {
                (int)((rect->x + pan_x) * scale),
                (int)((rect->y + pan_y) * scale),
                (int)(rect->width * scale),
                (int)(rect->height * scale)
            };
            
            int r_color = (rect->fill_color >> 16) & 0xFF;
            int g_color = (rect->fill_color >> 8) & 0xFF;
            int b_color = rect->fill_color & 0xFF;
            
            SDL_SetRenderDrawColor(renderer, r_color, g_color, b_color, 255);
            SDL_RenderFillRect(renderer, &sdl_rect);
            break;
        }
        
        case SVG_SHAPE_LINE: {
            SvgLine* line = &shape->data.line;
            int x1 = (int)((line->x1 + pan_x) * scale);
            int y1 = (int)((line->y1 + pan_y) * scale);
            int x2 = (int)((line->x2 + pan_x) * scale);
            int y2 = (int)((line->y2 + pan_y) * scale);
            
            int r_color = (line->stroke_color >> 16) & 0xFF;
            int g_color = (line->stroke_color >> 8) & 0xFF;
            int b_color = line->stroke_color & 0xFF;
            
            SDL_SetRenderDrawColor(renderer, r_color, g_color, b_color, 255);
            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
            break;
        }
    }
}

void render_selection_handles(SDL_Renderer* renderer, SvgShape* shape, double pan_x, double pan_y, double scale) {
    if (!shape) return;
    
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red selection handles
    
    switch (shape->type) {
        case SVG_SHAPE_CIRCLE: {
            SvgCircle* circle = &shape->data.circle;
            int cx = (int)((circle->cx + pan_x) * scale);
            int cy = (int)((circle->cy + pan_y) * scale);
            
            // Draw selection rectangle around circle
            SDL_Rect selection_rect = {
                cx - (int)(circle->r * scale) - 2,
                cy - (int)(circle->r * scale) - 2,
                (int)(circle->r * 2 * scale) + 4,
                (int)(circle->r * 2 * scale) + 4
            };
            SDL_RenderDrawRect(renderer, &selection_rect);
            break;
        }
        
        case SVG_SHAPE_RECT: {
            SvgRect* rect = &shape->data.rect;
            SDL_Rect selection_rect = {
                (int)((rect->x + pan_x) * scale) - 2,
                (int)((rect->y + pan_y) * scale) - 2,
                (int)(rect->width * scale) + 4,
                (int)(rect->height * scale) + 4
            };
            SDL_RenderDrawRect(renderer, &selection_rect);
            break;
        }
    }
}

void render_toolbar(SDL_Renderer* renderer, TTF_Font* font, SvgShape* selected_shape) {
    // Draw toolbar background
    SDL_Rect toolbar_rect = {0, 0, TOOLBAR_WIDTH, WINDOW_HEIGHT};
    SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
    SDL_RenderFillRect(renderer, &toolbar_rect);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderDrawLine(renderer, TOOLBAR_WIDTH, 0, TOOLBAR_WIDTH, WINDOW_HEIGHT);
    
    // TODO: Add toolbar buttons and information
    // This would include shape creation buttons, property editors, etc.
}