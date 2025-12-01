#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include "../include/svg_types.h"
#include "../include/svg_parser.h"
#include "../include/svg_render.h"
#include "../include/bmp_writer.h"
#include "../include/jpg_writer.h"

// 窗口设置
#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800
#define CANVAS_WIDTH 800
#define CANVAS_HEIGHT 600
#define TOOLBAR_WIDTH 200
#define PROPERTY_PANEL_WIDTH 200

// 颜色定义
#define COLOR_BACKGROUND 240, 240, 240
#define COLOR_CANVAS 255, 255, 255
#define COLOR_TOOLBAR 220, 220, 220
#define COLOR_PROPERTY 220, 220, 220
#define COLOR_BUTTON 180, 180, 180
#define COLOR_BUTTON_HOVER 160, 160, 160
#define COLOR_SELECTED 255, 0, 0
#define COLOR_GRID 230, 230, 230

// UI元素定义
typedef struct {
    SDL_Rect rect;
    const char* text;
    int active;
} Button;

typedef struct {
    SDL_Rect rect;
    const char* label;
    char value[64];
    int is_editing;
} TextInput;

typedef enum {
    TOOL_SELECT,
    TOOL_CIRCLE,
    TOOL_RECT,
    TOOL_LINE,
    TOOL_MOVE
} ToolType;

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    TTF_Font* small_font;

    // SVG文档
    SvgDocument* doc;
    char current_file[256];

    // UI状态
    ToolType current_tool;
    int selected_shape_id;
    int is_dragging;
    int drag_start_x, drag_start_y;
    int shape_start_x, shape_start_y;

    // 按钮和输入框
    Button buttons[10];
    int button_count;
    TextInput inputs[5];
    int input_count;

    // 菜单状态
    int show_file_menu;
    int show_help;

    // 画布偏移和缩放
    int canvas_offset_x, canvas_offset_y;
    float zoom_level;

} GUIState;

// 初始化SDL
int init_sdl(GUIState* gui) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL初始化失败: %s\n", SDL_GetError());
        return 0;
    }

    if (TTF_Init() < 0) {
        printf("TTF初始化失败: %s\n", TTF_GetError());
        SDL_Quit();
        return 0;
    }

    gui->window = SDL_CreateWindow("SVG 图形编辑器",
                                  SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED,
                                  WINDOW_WIDTH, WINDOW_HEIGHT,
                                  SDL_WINDOW_SHOWN);
    if (!gui->window) {
        printf("窗口创建失败: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 0;
    }

    gui->renderer = SDL_CreateRenderer(gui->window, -1, SDL_RENDERER_ACCELERATED);
    if (!gui->renderer) {
        printf("渲染器创建失败: %s\n", SDL_GetError());
        SDL_DestroyWindow(gui->window);
        TTF_Quit();
        SDL_Quit();
        return 0;
    }

    // 加载字体
    gui->font = TTF_OpenFont("C:/Windows/Fonts/msyh.ttc", 16); // 微软雅黑
    if (!gui->font) {
        gui->font = TTF_OpenFont("arial.ttf", 16);
    }
    gui->small_font = TTF_OpenFont("C:/Windows/Fonts/msyh.ttc", 12);
    if (!gui->small_font) {
        gui->small_font = TTF_OpenFont("arial.ttf", 12);
    }

    return 1;
}

// 初始化GUI状态
void init_gui_state(GUIState* gui) {
    gui->doc = create_svg_document(800, 600);
    gui->current_tool = TOOL_SELECT;
    gui->selected_shape_id = -1;
    gui->is_dragging = 0;
    gui->show_file_menu = 0;
    gui->show_help = 0;
    gui->canvas_offset_x = TOOLBAR_WIDTH;
    gui->canvas_offset_y = 50;
    gui->zoom_level = 1.0f;
    gui->current_file[0] = '\0';

    // 创建工具栏按钮
    gui->button_count = 0;

    // 工具按钮
    gui->buttons[gui->button_count++] = (Button){ {10, 60, 180, 30}, "选择工具", 0 };
    gui->buttons[gui->button_count++] = (Button){ {10, 100, 180, 30}, "圆形工具", 0 };
    gui->buttons[gui->button_count++] = (Button){ {10, 140, 180, 30}, "矩形工具", 0 };
    gui->buttons[gui->button_count++] = (Button){ {10, 180, 180, 30}, "直线工具", 0 };

    // 文件操作按钮
    gui->buttons[gui->button_count++] = (Button){ {10, 240, 85, 30}, "打开", 0 };
    gui->buttons[gui->button_count++] = (Button){ {105, 240, 85, 30}, "保存", 0 };
    gui->buttons[gui->button_count++] = (Button){ {10, 280, 85, 30}, "导出BMP", 0 };
    gui->buttons[gui->button_count++] = (Button){ {105, 280, 85, 30}, "导出JPG", 0 };
    gui->buttons[gui->button_count++] = (Button){ {10, 320, 85, 30}, "清空", 0 };
    gui->buttons[gui->button_count++] = (Button){ {105, 320, 85, 30}, "帮助", 0 };

    // 属性面板输入框
    gui->input_count = 0;
    gui->inputs[gui->input_count++] = (TextInput){ {CANVAS_WIDTH + TOOLBAR_WIDTH + 10, 60, 180, 25}, "X坐标:", "", 0 };
    gui->inputs[gui->input_count++] = (TextInput){ {CANVAS_WIDTH + TOOLBAR_WIDTH + 10, 95, 180, 25}, "Y坐标:", "", 0 };
    gui->inputs[gui->input_count++] = (TextInput){ {CANVAS_WIDTH + TOOLBAR_WIDTH + 10, 130, 180, 25}, "宽度/半径:", "", 0 };
    gui->inputs[gui->input_count++] = (TextInput){ {CANVAS_WIDTH + TOOLBAR_WIDTH + 10, 165, 180, 25}, "高度:", "", 0 };
    gui->inputs[gui->input_count++] = (TextInput){ {CANVAS_WIDTH + TOOLBAR_WIDTH + 10, 200, 180, 25}, "颜色:", "", 0 };
}

// 绘制按钮
void draw_button(GUIState* gui, Button* button) {
    SDL_Color color = button->active ? (SDL_Color){COLOR_BUTTON_HOVER} : (SDL_Color){COLOR_BUTTON};
    SDL_SetRenderDrawColor(gui->renderer, color.r, color.g, color.b, 255);
    SDL_RenderFillRect(gui->renderer, &button->rect);

    SDL_SetRenderDrawColor(gui->renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(gui->renderer, &button->rect);

    if (gui->font && button->text) {
        SDL_Color text_color = {0, 0, 0, 255};
        SDL_Surface* surface = TTF_RenderUTF8_Blended(gui->font, button->text, text_color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(gui->renderer, surface);
            if (texture) {
                SDL_Rect text_rect = {
                    button->rect.x + (button->rect.w - surface->w) / 2,
                    button->rect.y + (button->rect.h - surface->h) / 2,
                    surface->w, surface->h
                };
                SDL_RenderCopy(gui->renderer, texture, NULL, &text_rect);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    }
}

// 绘制输入框
void draw_text_input(GUIState* gui, TextInput* input) {
    // 背景
    SDL_SetRenderDrawColor(gui->renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(gui->renderer, &input->rect);

    // 边框
    SDL_SetRenderDrawColor(gui->renderer, input->is_editing ? 0 : 150,
                          input->is_editing ? 150 : 150,
                          input->is_editing ? 255 : 150, 255);
    SDL_RenderDrawRect(gui->renderer, &input->rect);

    // 标签
    if (gui->small_font && input->label) {
        SDL_Color text_color = {0, 0, 0, 255};
        SDL_Surface* surface = TTF_RenderUTF8_Blended(gui->small_font, input->label, text_color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(gui->renderer, surface);
            if (texture) {
                SDL_Rect text_rect = {
                    input->rect.x, input->rect.y - 20,
                    surface->w, surface->h
                };
                SDL_RenderCopy(gui->renderer, texture, NULL, &text_rect);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    }

    // 值
    if (gui->small_font && input->value[0]) {
        SDL_Color text_color = {0, 0, 0, 255};
        SDL_Surface* surface = TTF_RenderUTF8_Blended(gui->small_font, input->value, text_color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(gui->renderer, surface);
            if (texture) {
                SDL_Rect text_rect = {
                    input->rect.x + 5, input->rect.y + (input->rect.h - surface->h) / 2,
                    surface->w, surface->h
                };
                SDL_RenderCopy(gui->renderer, texture, NULL, &text_rect);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    }
}

// 绘制网格
void draw_grid(GUIState* gui) {
    SDL_SetRenderDrawColor(gui->renderer, COLOR_GRID, 255);

    int grid_size = 20;
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
void draw_svg_shapes(GUIState* gui) {
    if (!gui->doc) return;

    for (int i = 0; i < gui->doc->shape_count; i++) {
        SvgShape* shape = &gui->doc->shapes[i];

        // 设置颜色
        if (shape->type == SVG_SHAPE_LINE) {
            sscanf(shape->data.line.stroke, "#%02x%02x%02x",
                   &(int){0}, &(int){0}, &(int){0});
        } else {
            sscanf(shape->data.circle.fill, "#%02x%02x%02x",
                   &(int){0}, &(int){0}, &(int){0});
        }

        // 选中状态的边框
        if (shape->id == gui->selected_shape_id) {
            SDL_SetRenderDrawColor(gui->renderer, COLOR_SELECTED, 255);
            SDL_Rect outline;

            switch (shape->type) {
                case SVG_SHAPE_CIRCLE:
                    outline.x = gui->canvas_offset_x + (int)(shape->data.circle.cx - shape->data.circle.r - 5);
                    outline.y = gui->canvas_offset_y + (int)(shape->data.circle.cy - shape->data.circle.r - 5);
                    outline.w = (int)(shape->data.circle.r * 2 + 10);
                    outline.h = (int)(shape->data.circle.r * 2 + 10);
                    SDL_RenderDrawRect(gui->renderer, &outline);
                    break;
                case SVG_SHAPE_RECT:
                    outline.x = gui->canvas_offset_x + (int)(shape->data.rect.x - 5);
                    outline.y = gui->canvas_offset_y + (int)(shape->data.rect.y - 5);
                    outline.w = (int)(shape->data.rect.width + 10);
                    outline.h = (int)(shape->data.rect.height + 10);
                    SDL_RenderDrawRect(gui->renderer, &outline);
                    break;
            }
        }
    }
}

// 绘制界面
void draw_gui(GUIState* gui) {
    // 清空背景
    SDL_SetRenderDrawColor(gui->renderer, COLOR_BACKGROUND, 255);
    SDL_RenderClear(gui->renderer);

    // 绘制工具栏背景
    SDL_Rect toolbar = {0, 0, TOOLBAR_WIDTH, WINDOW_HEIGHT};
    SDL_SetRenderDrawColor(gui->renderer, COLOR_TOOLBAR, 255);
    SDL_RenderFillRect(gui->renderer, &toolbar);

    // 绘制属性面板背景
    SDL_Rect property_panel = {CANVAS_WIDTH + TOOLBAR_WIDTH, 0, PROPERTY_PANEL_WIDTH, WINDOW_HEIGHT};
    SDL_SetRenderDrawColor(gui->renderer, COLOR_PROPERTY, 255);
    SDL_RenderFillRect(gui->renderer, &property_panel);

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

    // 绘制输入框
    for (int i = 0; i < gui->input_count; i++) {
        draw_text_input(gui, &gui->inputs[i]);
    }

    // 绘制标题
    if (gui->font) {
        SDL_Color text_color = {0, 0, 0, 255};
        const char* title = gui->current_file[0] ? gui->current_file : "SVG 图形编辑器";
        SDL_Surface* surface = TTF_RenderUTF8_Blended(gui->font, title, text_color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(gui->renderer, surface);
            if (texture) {
                SDL_Rect text_rect = {
                    TOOLBAR_WIDTH + 10, 10, surface->w, surface->h
                };
                SDL_RenderCopy(gui->renderer, texture, NULL, &text_rect);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    }

    // 绘制工具栏标题
    if (gui->font) {
        SDL_Color text_color = {0, 0, 0, 255};
        SDL_Surface* surface = TTF_RenderUTF8_Blended(gui->font, "工具箱", text_color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(gui->renderer, surface);
            if (texture) {
                SDL_Rect text_rect = {10, 10, surface->w, surface->h};
                SDL_RenderCopy(gui->renderer, texture, NULL, &text_rect);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    }

    // 绘制属性面板标题
    if (gui->font) {
        SDL_Color text_color = {0, 0, 0, 255};
        SDL_Surface* surface = TTF_RenderUTF8_Blended(gui->font, "属性", text_color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(gui->renderer, surface);
            if (texture) {
                SDL_Rect text_rect = {CANVAS_WIDTH + TOOLBAR_WIDTH + 10, 10, surface->w, surface->h};
                SDL_RenderCopy(gui->renderer, texture, NULL, &text_rect);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    }

    SDL_RenderPresent(gui->renderer);
}

// 处理鼠标点击
void handle_mouse_click(GUIState* gui, int x, int y) {
    // 检查按钮点击
    for (int i = 0; i < gui->button_count; i++) {
        if (SDL_PointInRect(&(SDL_Point){x, y}, &gui->buttons[i].rect)) {
            switch (i) {
                case 0: gui->current_tool = TOOL_SELECT; break;
                case 1: gui->current_tool = TOOL_CIRCLE; break;
                case 2: gui->current_tool = TOOL_RECT; break;
                case 3: gui->current_tool = TOOL_LINE; break;
                case 4: // 打开文件
                    printf("打开文件功能待实现\n");
                    break;
                case 5: // 保存文件
                    printf("保存文件功能待实现\n");
                    break;
                case 6: // 导出BMP
                    if (gui->doc && gui->current_file[0]) {
                        char bmp_file[256];
                        strcpy(bmp_file, gui->current_file);
                        strcat(bmp_file, ".bmp");
                        if (export_to_bmp(gui->doc, bmp_file)) {
                            printf("已导出到 %s\n", bmp_file);
                        }
                    }
                    break;
                case 7: // 导出JPG
                    if (gui->doc && gui->current_file[0]) {
                        char jpg_file[256];
                        strcpy(jpg_file, gui->current_file);
                        strcat(jpg_file, ".jpg");
                        if (export_to_jpg(gui->doc, jpg_file)) {
                            printf("已导出到 %s\n", jpg_file);
                        }
                    }
                    break;
                case 8: // 清空画布
                    if (gui->doc) {
                        gui->doc->shape_count = 0;
                        gui->selected_shape_id = -1;
                    }
                    break;
                case 9: // 帮助
                    gui->show_help = !gui->show_help;
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
            for (int i = gui->doc->shape_count - 1; i >= 0; i--) {
                SvgShape* shape = &gui->doc->shapes[i];
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
                    update_property_inputs(gui, shape);
                    break;
                }
            }
        } else if (gui->current_tool == TOOL_CIRCLE) {
            // 添加圆形
            SvgShape new_shape;
            new_shape.type = SVG_SHAPE_CIRCLE;
            new_shape.id = gui->doc->shape_count + 1;
            new_shape.data.circle.cx = canvas_x;
            new_shape.data.circle.cy = canvas_y;
            new_shape.data.circle.r = 30;
            strcpy(new_shape.data.circle.fill, "#FF0000");

            if (gui->doc->shape_count < MAX_SHAPES) {
                gui->doc->shapes[gui->doc->shape_count++] = new_shape;
                gui->selected_shape_id = new_shape.id;
            }
        } else if (gui->current_tool == TOOL_RECT) {
            // 添加矩形
            SvgShape new_shape;
            new_shape.type = SVG_SHAPE_RECT;
            new_shape.id = gui->doc->shape_count + 1;
            new_shape.data.rect.x = canvas_x - 40;
            new_shape.data.rect.y = canvas_y - 30;
            new_shape.data.rect.width = 80;
            new_shape.data.rect.height = 60;
            strcpy(new_shape.data.rect.fill, "#0000FF");

            if (gui->doc->shape_count < MAX_SHAPES) {
                gui->doc->shapes[gui->doc->shape_count++] = new_shape;
                gui->selected_shape_id = new_shape.id;
            }
        }
    }
}

// 更新属性输入框
void update_property_inputs(GUIState* gui, SvgShape* shape) {
    if (!shape) return;

    switch (shape->type) {
        case SVG_SHAPE_CIRCLE:
            sprintf(gui->inputs[0].value, "%.1f", shape->data.circle.cx);
            sprintf(gui->inputs[1].value, "%.1f", shape->data.circle.cy);
            sprintf(gui->inputs[2].value, "%.1f", shape->data.circle.r);
            strcpy(gui->inputs[4].value, shape->data.circle.fill);
            gui->inputs[3].value[0] = '\0'; // 高度不适用于圆形
            break;
        case SVG_SHAPE_RECT:
            sprintf(gui->inputs[0].value, "%.1f", shape->data.rect.x);
            sprintf(gui->inputs[1].value, "%.1f", shape->data.rect.y);
            sprintf(gui->inputs[2].value, "%.1f", shape->data.rect.width);
            sprintf(gui->inputs[3].value, "%.1f", shape->data.rect.height);
            strcpy(gui->inputs[4].value, shape->data.rect.fill);
            break;
        case SVG_SHAPE_LINE:
            sprintf(gui->inputs[0].value, "%.1f", shape->data.line.x1);
            sprintf(gui->inputs[1].value, "%.1f", shape->data.line.y1);
            sprintf(gui->inputs[2].value, "%.1f", shape->data.line.x2);
            sprintf(gui->inputs[3].value, "%.1f", shape->data.line.y2);
            strcpy(gui->inputs[4].value, shape->data.line.stroke);
            break;
    }
}

// 主循环
int main() {
    GUIState gui = {0};

    if (!init_sdl(&gui)) {
        return 1;
    }

    init_gui_state(&gui);

    printf("=== SVG 图形编辑器 ===\n");
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
                        for (int i = 0; i < gui.doc->shape_count; i++) {
                            if (gui.doc->shapes[i].id == gui.selected_shape_id) {
                                SvgShape* shape = &gui.doc->shapes[i];

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

                                update_property_inputs(&gui, shape);
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
    if (gui.doc) {
        free(gui.doc);
    }
    if (gui.font) {
        TTF_CloseFont(gui.font);
    }
    if (gui.small_font) {
        TTF_CloseFont(gui.small_font);
    }
    SDL_DestroyRenderer(gui.renderer);
    SDL_DestroyWindow(gui.window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}