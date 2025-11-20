#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "../include/svg_types.h"
#include "../include/svg_parser.h"
#include "../include/gui_render.h"

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800
#define TOOLBAR_WIDTH 200
#define CANVAS_COLOR 0xF0F0F0

GuiContext* gui_create() {
    GuiContext* ctx = malloc(sizeof(GuiContext));
    if (!ctx) return NULL;
    
    // Initialize SDL video subsystem
    // 初始化SDL视频子系统
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        free(ctx);
        return NULL;
    }
    
    // Initialize SDL_ttf for font rendering
    // 初始化SDL_ttf用于字体渲染
    if (TTF_Init() == -1) {
        SDL_Quit();
        free(ctx);
        return NULL;
    }
    
    // Create the main application window
    // 创建主应用窗口
    ctx->window = SDL_CreateWindow("SVG Editor - ENGR1010J",
                                  SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED,
                                  WINDOW_WIDTH, WINDOW_HEIGHT,
                                  SDL_WINDOW_RESIZABLE);
    
    if (!ctx->window) {
        TTF_Quit();
        SDL_Quit();
        free(ctx);
        return NULL;
    }
    
    // Create the renderer for the window
    // 为窗口创建渲染器
    ctx->renderer = SDL_CreateRenderer(ctx->window, -1, 
                                      SDL_RENDERER_ACCELERATED);
    ctx->font = TTF_OpenFont("arial.ttf", 16);
    ctx->document = NULL;
    ctx->selected_shape = NULL;
    ctx->zoom_level = 100;
    ctx->pan_x = ctx->pan_y = 0;
    ctx->is_dragging = 0;
    
    return ctx;
}