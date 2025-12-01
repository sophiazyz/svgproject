#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL.h>

// 简化的GUI版本，不依赖TTF和libjpeg
// 窗口设置
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 600
#define CANVAS_WIDTH 700
#define CANVAS_HEIGHT 500
#define TOOLBAR_WIDTH 200

// 颜色定义
#define COLOR_BACKGROUND 240, 240, 240
#define COLOR_CANVAS 255, 255, 255
#define COLOR_TOOLBAR 220, 220, 220
#define COLOR_BUTTON 180, 180, 180
#define COLOR_SELECTED 255, 0, 0
#define COLOR_GRID 230, 230, 230

// 简化的图形结构
#define MAX_SHAPES 100

typedef enum {
    SVG_SHAPE_CIRCLE,
    SVG_SHAPE_RECT,
    SVG_SHAPE_LINE
} SvgShapeType;

typedef struct {
    double cx, cy, r;
    int r_color, g_color, b_color;
} SvgCircle;

typedef struct {
    double x, y, width, height;
    int r_color, g_color, b_color;
} SvgRect;

typedef struct {
    double x1, y1, x2, y2;
    int r_color, g_color, b_color;
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

typedef struct {
    SDL_Rect rect;
    const char* text;
    int active;
} Button;

typedef enum {
    TOOL_SELECT,
    TOOL_CIRCLE,
    TOOL_RECT,
    TOOL_LINE
} ToolType;

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;

    // SVG文档
    SvgDocument doc;

    // UI状态
    ToolType current_tool;
    int selected_shape_id;
    int is_dragging;
    int drag_start_x, drag_start_y;

    // 按钮
    Button buttons[10];
    int button_count;

    // 画布偏移
    int canvas_offset_x, canvas_offset_y;

} SimpleGUIState;

// 创建SVG文档
void init_svg_document(SvgDocument* doc, double width, double height) {
    doc->width = width;
    doc->height = height;
    doc->shape_count = 0;
}

// 解析颜色
void parse_color(const char* color_str, int* r, int* g, int* b) {
    if (!color_str || !r || !g || !b) {
        *r = *g = *b = 0;
        return;
    }

    if (color_str[0] == '#') {
        unsigned int color;
        sscanf(color_str + 1, "%06x", &color);
        *r = (color >> 16) & 0xFF;
        *g = (color >> 8) & 0xFF;
        *b = color & 0xFF;
        return;
    }

    // 预定义颜色
    if (strcmp(color_str, "red") == 0) { *r = 255; *g = 0; *b = 0; }
    else if (strcmp(color_str, "green") == 0) { *r = 0; *g = 255; *b = 0; }
    else if (strcmp(color_str, "blue") == 0) { *r = 0; *g = 0; *b = 255; }
    else if (strcmp(color_str, "yellow") == 0) { *r = 255; *g = 255; *b = 0; }
    else if (strcmp(color_str, "black") == 0) { *r = 0; *g = 0; *b = 0; }
    else if (strcmp(color_str, "white") == 0) { *r = 255; *g = 255; *b = 255; }
    else { *r = 0; *g = 0; *b = 0; } // 默认黑色
}

// 绘制像素文本（简单的字符绘制）
void draw_simple_text(SDL_Renderer* renderer, const char* text, int x, int y) {
    // 简单的矩形绘制来代替文本
    for (int i = 0; text[i] && i < 10; i++) {
        SDL_Rect char_rect = {x + i * 8, y, 6, 8};
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &char_rect);
    }
}

// 初始化SDL
int init_sdl(SimpleGUIState* gui) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL初始化失败: %s\n", SDL_GetError());
        return 0;
    }

    gui->window = SDL_CreateWindow("SVG 简单图形编辑器",
                                  SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED,
                                  WINDOW_WIDTH, WINDOW_HEIGHT,
                                  SDL_WINDOW_SHOWN);
    if (!gui->window) {
        printf("窗口创建失败: %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    gui->renderer = SDL_CreateRenderer(gui->window, -1, SDL_RENDERER_ACCELERATED);
    if (!gui->renderer) {
        printf("渲染器创建失败: %s\n", SDL_GetError());
        SDL_DestroyWindow(gui->window);
        SDL_Quit();
        return 0;
    }

    return 1;
}

// 初始化GUI状态
void init_gui_state(SimpleGUIState* gui) {
    init_svg_document(&gui->doc, CANVAS_WIDTH, CANVAS_HEIGHT);
    gui->current_tool = TOOL_SELECT;
    gui->selected_shape_id = -1;
    gui->is_dragging = 0;
    gui->canvas_offset_x = TOOLBAR_WIDTH;
    gui->canvas_offset_y = 50;

    // 创建工具栏按钮
    gui->button_count = 0;
    gui->buttons[gui->button_count++] = (Button){ {10, 60, 180, 30}, "Select", 0 };
    gui->buttons[gui->button_count++] = (Button){ {10, 100, 180, 30}, "Circle", 0 };
    gui->buttons[gui->button_count++] = (Button){ {10, 140, 180, 30}, "Rect", 0 };
    gui->buttons[gui->button_count++] = (Button){ {10, 180, 180, 30}, "Line", 0 };
    gui->buttons[gui->button_count++] = (Button){ {10, 240, 180, 30}, "Clear", 0 };
}

// 绘制按钮
void draw_button(SimpleGUIState* gui, Button* button) {
    SDL_Color color = button->active ? (SDL_Color){160, 160, 160, 255} : (SDL_Color){180, 180, 180, 255};
    SDL_SetRenderDrawColor(gui->renderer, color.r, color.g, color.b, 255);
    SDL_RenderFillRect(gui->renderer, &button->rect);

    SDL_SetRenderDrawColor(gui->renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(gui->renderer, &button->rect);

    draw_simple_text(gui->renderer, button->text, button->rect.x + 10, button->rect.y + 10);
}

// 绘制网格
void draw_grid(SimpleGUIState* gui) {
    SDL_SetRenderDrawColor(gui->renderer, COLOR_GRID, 255);

    int grid_size = 25;
    for (int x = gui->canvas_offset_x; x < gui->canvas_offset_x + CANVAS_WIDTH; x += grid_size) {
        SDL_RenderDrawLine(gui->renderer, x, gui->canvas_offset_y,
                          x, gui->canvas_offset_y + CANVAS_HEIGHT);
    }
    for (int y = gui->canvas_offset_y; y < gui->canvas_offset_y + CANVAS_HEIGHT; y += grid_size) {
        SDL_RenderDrawLine(gui->renderer, gui->canvas_offset_x, y,
                          gui->canvas_offset_x + CANVAS_WIDTH, y);
    }
}

// 绘制SVG图形
void draw_svg_shapes(SimpleGUIState* gui) {
    for (int i = 0; i < gui->doc.shape_count; i++) {
        SvgShape* shape = &gui->doc.shapes[i];

        switch (shape->type) {
            case SVG_SHAPE_CIRCLE:
                SDL_SetRenderDrawColor(gui->renderer,
                                      shape->data.circle.r_color,
                                      shape->data.circle.g_color,
                                      shape->data.circle.b_color, 255);

                // 简单的圆形绘制（使用多个小矩形近似）
                int radius = (int)shape->data.circle.r;
                int cx = (int)shape->data.circle.cx + gui->canvas_offset_x;
                int cy = (int)shape->data.circle.cy + gui->canvas_offset_y;

                for (int y = -radius; y <= radius; y++) {
                    for (int x = -radius; x <= radius; x++) {
                        if (x*x + y*y <= radius*radius) {
                            SDL_RenderDrawPoint(gui->renderer, cx + x, cy + y);
                        }
                    }
                }
                break;

            case SVG_SHAPE_RECT:
                SDL_SetRenderDrawColor(gui->renderer,
                                      shape->data.rect.r_color,
                                      shape->data.rect.g_color,
                                      shape->data.rect.b_color, 255);

                SDL_Rect rect = {
                    (int)shape->data.rect.x + gui->canvas_offset_x,
                    (int)shape->data.rect.y + gui->canvas_offset_y,
                    (int)shape->data.rect.width,
                    (int)shape->data.rect.height
                };
                SDL_RenderFillRect(gui->renderer, &rect);
                break;

            case SVG_SHAPE_LINE:
                SDL_SetRenderDrawColor(gui->renderer,
                                      shape->data.line.r_color,
                                      shape->data.line.g_color,
                                      shape->data.line.b_color, 255);

                SDL_RenderDrawLine(gui->renderer,
                                  (int)shape->data.line.x1 + gui->canvas_offset_x,
                                  (int)shape->data.line.y1 + gui->canvas_offset_y,
                                  (int)shape->data.line.x2 + gui->canvas_offset_x,
                                  (int)shape->data.line.y2 + gui->canvas_offset_y);
                break;
        }

        // 绘制选中边框
        if (shape->id == gui->selected_shape_id) {
            SDL_SetRenderDrawColor(gui->renderer, COLOR_SELECTED, 255);
            SDL_Rect outline;

            switch (shape->type) {
                case SVG_SHAPE_CIRCLE:
                    {
                        int r = (int)shape->data.circle.r;
                        int cx = (int)shape->data.circle.cx;
                        int cy = (int)shape->data.circle.cy;
                        outline.x = gui->canvas_offset_x + cx - r - 5;
                        outline.y = gui->canvas_offset_y + cy - r - 5;
                        outline.w = r * 2 + 10;
                        outline.h = r * 2 + 10;
                    }
                    break;
                case SVG_SHAPE_RECT:
                    outline.x = gui->canvas_offset_x + (int)shape->data.rect.x - 5;
                    outline.y = gui->canvas_offset_y + (int)shape->data.rect.y - 5;
                    outline.w = (int)shape->data.rect.width + 10;
                    outline.h = (int)shape->data.rect.height + 10;
                    break;
                case SVG_SHAPE_LINE:
                    outline.x = gui->canvas_offset_x + (int)fmin(shape->data.line.x1, shape->data.line.x2) - 5;
                    outline.y = gui->canvas_offset_y + (int)fmin(shape->data.line.y1, shape->data.line.y2) - 5;
                    outline.w = (int)fabs(shape->data.line.x2 - shape->data.line.x1) + 10;
                    outline.h = (int)fabs(shape->data.line.y2 - shape->data.line.y1) + 10;
                    break;
            }
            SDL_RenderDrawRect(gui->renderer, &outline);
        }
    }
}

// 绘制界面
void draw_gui(SimpleGUIState* gui) {
    // 清空背景
    SDL_SetRenderDrawColor(gui->renderer, COLOR_BACKGROUND, 255);
    SDL_RenderClear(gui->renderer);

    // 绘制工具栏背景
    SDL_Rect toolbar = {0, 0, TOOLBAR_WIDTH, WINDOW_HEIGHT};
    SDL_SetRenderDrawColor(gui->renderer, COLOR_TOOLBAR, 255);
    SDL_RenderFillRect(gui->renderer, &toolbar);

    // 绘制画布背景
    SDL_Rect canvas = {gui->canvas_offset_x, gui->canvas_offset_y, CANVAS_WIDTH, CANVAS_HEIGHT};
    SDL_SetRenderDrawColor(gui->renderer, COLOR_CANVAS, 255);
    SDL_RenderFillRect(gui->renderer, &canvas);

    // 绘制网格
    draw_grid(gui);

    // 绘制SVG图形
    draw_svg_shapes(gui);

    // 绘制按钮
    for (int i = 0; i < gui->button_count; i++) {
        draw_button(gui, &gui->buttons[i]);
    }

    // 绘制标题
    draw_simple_text(gui->renderer, "SVG Simple Editor", TOOLBAR_WIDTH + 10, 10);

    SDL_RenderPresent(gui->renderer);
}

// 处理鼠标点击
void handle_mouse_click(SimpleGUIState* gui, int x, int y) {
    // 检查按钮点击
    for (int i = 0; i < gui->button_count; i++) {
        if (SDL_PointInRect(&(SDL_Point){x, y}, &gui->buttons[i].rect)) {
            switch (i) {
                case 0: gui->current_tool = TOOL_SELECT; break;
                case 1: gui->current_tool = TOOL_CIRCLE; break;
                case 2: gui->current_tool = TOOL_RECT; break;
                case 3: gui->current_tool = TOOL_LINE; break;
                case 4: // Clear
                    gui->doc.shape_count = 0;
                    gui->selected_shape_id = -1;
                    break;
            }
            return;
        }
    }

    // 检查画布点击
    if (x >= gui->canvas_offset_x && x < gui->canvas_offset_x + CANVAS_WIDTH &&
        y >= gui->canvas_offset_y && y < gui->canvas_offset_y + CANVAS_HEIGHT) {

        int canvas_x = x - gui->canvas_offset_x;
        int canvas_y = y - gui->canvas_offset_y;

        if (gui->current_tool == TOOL_SELECT) {
            // 选择图形
            gui->selected_shape_id = -1;
            for (int i = gui->doc.shape_count - 1; i >= 0; i--) {
                SvgShape* shape = &gui->doc.shapes[i];
                int is_hit = 0;

                switch (shape->type) {
                    case SVG_SHAPE_CIRCLE:
                        {
                            double dx = canvas_x - shape->data.circle.cx;
                            double dy = canvas_y - shape->data.circle.cy;
                            if (sqrt(dx*dx + dy*dy) <= shape->data.circle.r) {
                                is_hit = 1;
                            }
                        }
                        break;
                    case SVG_SHAPE_RECT:
                        if (canvas_x >= shape->data.rect.x &&
                            canvas_x <= shape->data.rect.x + shape->data.rect.width &&
                            canvas_y >= shape->data.rect.y &&
                            canvas_y <= shape->data.rect.y + shape->data.rect.height) {
                            is_hit = 1;
                        }
                        break;
                }

                if (is_hit) {
                    gui->selected_shape_id = shape->id;
                    gui->is_dragging = 1;
                    gui->drag_start_x = canvas_x;
                    gui->drag_start_y = canvas_y;
                    break;
                }
            }
        } else if (gui->current_tool == TOOL_CIRCLE) {
            // 添加圆形
            if (gui->doc.shape_count < MAX_SHAPES) {
                SvgShape new_shape;
                new_shape.type = SVG_SHAPE_CIRCLE;
                new_shape.id = gui->doc.shape_count + 1;
                new_shape.data.circle.cx = canvas_x;
                new_shape.data.circle.cy = canvas_y;
                new_shape.data.circle.r = 30;
                new_shape.data.circle.r_color = 255;
                new_shape.data.circle.g_color = 0;
                new_shape.data.circle.b_color = 0;

                gui->doc.shapes[gui->doc.shape_count++] = new_shape;
                gui->selected_shape_id = new_shape.id;
            }
        } else if (gui->current_tool == TOOL_RECT) {
            // 添加矩形
            if (gui->doc.shape_count < MAX_SHAPES) {
                SvgShape new_shape;
                new_shape.type = SVG_SHAPE_RECT;
                new_shape.id = gui->doc.shape_count + 1;
                new_shape.data.rect.x = canvas_x - 40;
                new_shape.data.rect.y = canvas_y - 30;
                new_shape.data.rect.width = 80;
                new_shape.data.rect.height = 60;
                new_shape.data.rect.r_color = 0;
                new_shape.data.rect.g_color = 0;
                new_shape.data.rect.b_color = 255;

                gui->doc.shapes[gui->doc.shape_count++] = new_shape;
                gui->selected_shape_id = new_shape.id;
            }
        } else if (gui->current_tool == TOOL_LINE) {
            // 添加直线（简单的水平线）
            if (gui->doc.shape_count < MAX_SHAPES) {
                SvgShape new_shape;
                new_shape.type = SVG_SHAPE_LINE;
                new_shape.id = gui->doc.shape_count + 1;
                new_shape.data.line.x1 = canvas_x - 50;
                new_shape.data.line.y1 = canvas_y;
                new_shape.data.line.x2 = canvas_x + 50;
                new_shape.data.line.y2 = canvas_y;
                new_shape.data.line.r_color = 0;
                new_shape.data.line.g_color = 128;
                new_shape.data.line.b_color = 0;

                gui->doc.shapes[gui->doc.shape_count++] = new_shape;
                gui->selected_shape_id = new_shape.id;
            }
        }
    }
}

// 主循环
int main() {
    SimpleGUIState gui = {0};

    if (!init_sdl(&gui)) {
        return 1;
    }

    init_gui_state(&gui);

    printf("=== SVG 简单图形编辑器 ===\n");
    printf("使用鼠标点击工具栏选择工具\n");
    printf("在画布上点击添加图形或选择现有图形\n");
    printf("按 ESC 或关闭窗口退出\n");

    int running = 1;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = 0;
                    break;

                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        running = 0;
                    }
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        handle_mouse_click(&gui, event.button.x, event.button.y);
                    }
                    break;

                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        gui.is_dragging = 0;
                    }
                    break;

                case SDL_MOUSEMOTION:
                    if (gui.is_dragging && gui.selected_shape_id != -1) {
                        int canvas_x = event.motion.x - gui.canvas_offset_x;
                        int canvas_y = event.motion.y - gui.canvas_offset_y;
                        int dx = canvas_x - gui.drag_start_x;
                        int dy = canvas_y - gui.drag_start_y;

                        // 移动选中的图形
                        for (int i = 0; i < gui.doc.shape_count; i++) {
                            if (gui.doc.shapes[i].id == gui.selected_shape_id) {
                                SvgShape* shape = &gui.doc.shapes[i];

                                switch (shape->type) {
                                    case SVG_SHAPE_CIRCLE:
                                        shape->data.circle.cx += dx;
                                        shape->data.circle.cy += dy;
                                        break;
                                    case SVG_SHAPE_RECT:
                                        shape->data.rect.x += dx;
                                        shape->data.rect.y += dy;
                                        break;
                                    case SVG_SHAPE_LINE:
                                        shape->data.line.x1 += dx;
                                        shape->data.line.y1 += dy;
                                        shape->data.line.x2 += dx;
                                        shape->data.line.y2 += dy;
                                        break;
                                }
                                break;
                            }
                        }

                        gui.drag_start_x = canvas_x;
                        gui.drag_start_y = canvas_y;
                    }
                    break;
            }
        }

        draw_gui(&gui);
        SDL_Delay(16); // 约60 FPS
    }

    // 清理资源
    SDL_DestroyRenderer(gui.renderer);
    SDL_DestroyWindow(gui.window);
    SDL_Quit();

    return 0;
}