#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <windows.h>
#include <windowsx.h>

// 窗口设置
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 700
#define CANVAS_WIDTH 700
#define CANVAS_HEIGHT 500
#define TOOLBAR_WIDTH 200
#define PROPERTY_PANEL_WIDTH 100

// ID定义
#define ID_BUTTON_SELECT    1001
#define ID_BUTTON_CIRCLE    1002
#define ID_BUTTON_RECT      1003
#define ID_BUTTON_LINE      1004
#define ID_BUTTON_CLEAR     1005
#define ID_BUTTON_EXPORT    1006
#define ID_BUTTON_EXIT      1007

// 图形结构
#define MAX_SHAPES 100

typedef enum {
    SVG_SHAPE_CIRCLE,
    SVG_SHAPE_RECT,
    SVG_SHAPE_LINE
} SvgShapeType;

typedef struct {
    double cx, cy, r;
    COLORREF color;
} SvgCircle;

typedef struct {
    double x, y, width, height;
    COLORREF color;
} SvgRect;

typedef struct {
    double x1, y1, x2, y2;
    COLORREF color;
} SvgLine;

typedef struct {
    SvgShapeType type;
    union {
        SvgCircle circle;
        SvgRect rect;
        SvgLine line;
    } data;
    int id;
    BOOL selected;
} SvgShape;

typedef struct {
    double width, height;
    SvgShape shapes[MAX_SHAPES];
    int shape_count;
    int selected_shape_id;
} SvgDocument;

typedef enum {
    TOOL_SELECT,
    TOOL_CIRCLE,
    TOOL_RECT,
    TOOL_LINE
} ToolType;

typedef struct {
    HWND hwnd;
    HDC hdc;
    HBITMAP canvas_bitmap;
    HBITMAP old_bitmap;
    HDC canvas_dc;

    // SVG文档
    SvgDocument doc;

    // UI状态
    ToolType current_tool;
    int is_dragging;
    int drag_start_x, drag_start_y;
    POINT last_mouse_pos;

    // 控件句柄
    HWND button_select;
    HWND button_circle;
    HWND button_rect;
    HWND button_line;
    HWND button_clear;
    HWND button_export;
    HWND button_exit;
    HWND status_text;

} Win32GUI;

// 函数声明
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void init_gui_controls(HWND hwnd, Win32GUI* gui);
void draw_canvas(Win32GUI* gui);
void draw_shape(Win32GUI* gui, SvgShape* shape);
void handle_canvas_click(Win32GUI* gui, int x, int y);
void update_dragging(Win32GUI* gui, int x, int y);
void clear_selection(Win32GUI* gui);
void select_shape(Win32GUI* gui, int shape_id);
void add_circle(Win32GUI* gui, int x, int y);
void add_rect(Win32GUI* gui, int x, int y);
void add_line(Win32GUI* gui, int x, int y);
void clear_all_shapes(Win32GUI* gui);
void export_svg(Win32GUI* gui);
void update_status(Win32GUI* gui, const char* text);

// 解析颜色
COLORREF parse_color(const char* color_str) {
    if (!color_str) return RGB(0, 0, 0);

    // 十六进制颜色
    if (color_str[0] == '#') {
        unsigned int color;
        sscanf(color_str + 1, "%06x", &color);
        return RGB((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
    }

    if (strcmp(color_str, "red") == 0) return RGB(255, 0, 0);
    if (strcmp(color_str, "green") == 0) return RGB(0, 255, 0);
    if (strcmp(color_str, "blue") == 0) return RGB(0, 0, 255);
    if (strcmp(color_str, "yellow") == 0) return RGB(255, 255, 0);
    if (strcmp(color_str, "orange") == 0) return RGB(255, 165, 0);
    if (strcmp(color_str, "purple") == 0) return RGB(128, 0, 128);
    if (strcmp(color_str, "pink") == 0) return RGB(255, 192, 203);
    if (strcmp(color_str, "white") == 0) return RGB(255, 255, 255);
    if (strcmp(color_str, "black") == 0) return RGB(0, 0, 0);

    return RGB(0, 0, 0); // 默认黑色
}

// 初始化GUI控件
void init_gui_controls(HWND hwnd, Win32GUI* gui) {
    // 创建按钮
    gui->button_select = CreateWindow(
        "BUTTON", "选择",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        10, 10, 180, 30,
        hwnd, (HMENU)ID_BUTTON_SELECT, NULL, NULL
    );

    gui->button_circle = CreateWindow(
        "BUTTON", "圆形",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        10, 50, 180, 30,
        hwnd, (HMENU)ID_BUTTON_CIRCLE, NULL, NULL
    );

    gui->button_rect = CreateWindow(
        "BUTTON", "矩形",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        10, 90, 180, 30,
        hwnd, (HMENU)ID_BUTTON_RECT, NULL, NULL
    );

    gui->button_line = CreateWindow(
        "BUTTON", "直线",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        10, 130, 180, 30,
        hwnd, (HMENU)ID_BUTTON_LINE, NULL, NULL
    );

    gui->button_clear = CreateWindow(
        "BUTTON", "清空",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        10, 180, 85, 30,
        hwnd, (HMENU)ID_BUTTON_CLEAR, NULL, NULL
    );

    gui->button_export = CreateWindow(
        "BUTTON", "导出SVG",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        105, 180, 85, 30,
        hwnd, (HMENU)ID_BUTTON_EXPORT, NULL, NULL
    );

    gui->button_exit = CreateWindow(
        "BUTTON", "退出",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        10, 220, 180, 30,
        hwnd, (HMENU)ID_BUTTON_EXIT, NULL, NULL
    );

    // 状态文本
    gui->status_text = CreateWindow(
        "STATIC", "SVG 图形编辑器 - 点击工具开始绘制",
        WS_VISIBLE | WS_CHILD | SS_LEFT,
        TOOLBAR_WIDTH + 10, WINDOW_HEIGHT - 30, WINDOW_WIDTH - TOOLBAR_WIDTH - 20, 20,
        hwnd, NULL, NULL, NULL
    );

    // 设置字体
    HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Microsoft YaHei");

    SendMessage(gui->button_select, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(gui->button_circle, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(gui->button_rect, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(gui->button_line, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(gui->button_clear, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(gui->button_export, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(gui->button_exit, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(gui->status_text, WM_SETFONT, (WPARAM)hFont, TRUE);
}

// 绘制单个图形
void draw_shape(Win32GUI* gui, SvgShape* shape) {
    HBRUSH brush;
    HPEN pen;

    switch (shape->type) {
        case SVG_SHAPE_CIRCLE:
            brush = CreateSolidBrush(shape->data.circle.color);
            pen = CreatePen(PS_SOLID, 1, shape->data.circle.color);
            SelectObject(gui->canvas_dc, brush);
            SelectObject(gui->canvas_dc, pen);
            Ellipse(gui->canvas_dc,
                    (int)(shape->data.circle.cx - shape->data.circle.r),
                    (int)(shape->data.circle.cy - shape->data.circle.r),
                    (int)(shape->data.circle.cx + shape->data.circle.r),
                    (int)(shape->data.circle.cy + shape->data.circle.r));
            DeleteObject(brush);
            DeleteObject(pen);
            break;

        case SVG_SHAPE_RECT:
            brush = CreateSolidBrush(shape->data.rect.color);
            pen = CreatePen(PS_SOLID, 1, shape->data.rect.color);
            SelectObject(gui->canvas_dc, brush);
            SelectObject(gui->canvas_dc, pen);
            Rectangle(gui->canvas_dc,
                      (int)shape->data.rect.x, (int)shape->data.rect.y,
                      (int)(shape->data.rect.x + shape->data.rect.width),
                      (int)(shape->data.rect.y + shape->data.rect.height));
            DeleteObject(brush);
            DeleteObject(pen);
            break;

        case SVG_SHAPE_LINE:
            pen = CreatePen(PS_SOLID, 2, shape->data.line.color);
            SelectObject(gui->canvas_dc, pen);
            MoveToEx(gui->canvas_dc, (int)shape->data.line.x1, (int)shape->data.line.y1, NULL);
            LineTo(gui->canvas_dc, (int)shape->data.line.x2, (int)shape->data.line.y2);
            DeleteObject(pen);
            break;
    }

    // 绘制选中边框
    if (shape->selected) {
        HPEN red_pen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
        SelectObject(gui->canvas_dc, red_pen);
        SetROP2(gui->canvas_dc, R2_NOTXORPEN);

        switch (shape->type) {
            case SVG_SHAPE_CIRCLE:
                {
                    int r = (int)shape->data.circle.r + 5;
                    Ellipse(gui->canvas_dc,
                            (int)(shape->data.circle.cx - r),
                            (int)(shape->data.circle.cy - r),
                            (int)(shape->data.circle.cx + r),
                            (int)(shape->data.circle.cy + r));
                }
                break;
            case SVG_SHAPE_RECT:
                Rectangle(gui->canvas_dc,
                          (int)shape->data.rect.x - 5, (int)shape->data.rect.y - 5,
                          (int)(shape->data.rect.x + shape->data.rect.width + 5),
                          (int)(shape->data.rect.y + shape->data.rect.height + 5));
                break;
            case SVG_SHAPE_LINE:
                {
                    int x1 = (int)shape->data.line.x1;
                    int y1 = (int)shape->data.line.y1;
                    int x2 = (int)shape->data.line.x2;
                    int y2 = (int)shape->data.line.y2;
                    Rectangle(gui->canvas_dc,
                              min(x1, x2) - 5, min(y1, y2) - 5,
                              max(x1, x2) + 5, max(y1, y2) + 5);
                }
                break;
        }

        SetROP2(gui->canvas_dc, R2_COPYPEN);
        DeleteObject(red_pen);
    }
}

// 绘制画布
void draw_canvas(Win32GUI* gui) {
    if (!gui->canvas_dc) return;

    // 清空画布为白色
    RECT canvas_rect = {0, 0, CANVAS_WIDTH, CANVAS_HEIGHT};
    HBRUSH white_brush = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(gui->canvas_dc, &canvas_rect, white_brush);
    DeleteObject(white_brush);

    // 绘制网格
    HPEN grid_pen = CreatePen(PS_DOT, 1, RGB(200, 200, 200));
    SelectObject(gui->canvas_dc, grid_pen);
    for (int x = 0; x <= CANVAS_WIDTH; x += 25) {
        MoveToEx(gui->canvas_dc, x, 0, NULL);
        LineTo(gui->canvas_dc, x, CANVAS_HEIGHT);
    }
    for (int y = 0; y <= CANVAS_HEIGHT; y += 25) {
        MoveToEx(gui->canvas_dc, 0, y, NULL);
        LineTo(gui->canvas_dc, CANVAS_WIDTH, y);
    }
    DeleteObject(grid_pen);

    // 绘制所有图形
    for (int i = 0; i < gui->doc.shape_count; i++) {
        draw_shape(gui, &gui->doc.shapes[i]);
    }
}

// 清空选择
void clear_selection(Win32GUI* gui) {
    for (int i = 0; i < gui->doc.shape_count; i++) {
        gui->doc.shapes[i].selected = FALSE;
    }
    gui->doc.selected_shape_id = -1;
}

// 选择图形
void select_shape(Win32GUI* gui, int shape_id) {
    clear_selection(gui);
    if (shape_id >= 0 && shape_id < gui->doc.shape_count) {
        gui->doc.shapes[shape_id].selected = TRUE;
        gui->doc.selected_shape_id = shape_id;

        char status[256];
        SvgShape* shape = &gui->doc.shapes[shape_id];
        switch (shape->type) {
            case SVG_SHAPE_CIRCLE:
                sprintf(status, "选中圆形 #%d: 中心(%.1f,%.1f), 半径%.1f",
                        shape_id + 1, shape->data.circle.cx, shape->data.circle.cy, shape->data.circle.r);
                break;
            case SVG_SHAPE_RECT:
                sprintf(status, "选中矩形 #%d: 位置(%.1f,%.1f), 大小%.1fx%.1f",
                        shape_id + 1, shape->data.rect.x, shape->data.rect.y,
                        shape->data.rect.width, shape->data.rect.height);
                break;
            case SVG_SHAPE_LINE:
                sprintf(status, "选中直线 #%d: 从(%.1f,%.1f) 到(%.1f,%.1f)",
                        shape_id + 1, shape->data.line.x1, shape->data.line.y1,
                        shape->data.line.x2, shape->data.line.y2);
                break;
        }
        update_status(gui, status);
    }
}

// 添加圆形
void add_circle(Win32GUI* gui, int x, int y) {
    if (gui->doc.shape_count < MAX_SHAPES) {
        SvgShape* shape = &gui->doc.shapes[gui->doc.shape_count];
        shape->type = SVG_SHAPE_CIRCLE;
        shape->id = gui->doc.shape_count + 1;
        shape->data.circle.cx = x;
        shape->data.circle.cy = y;
        shape->data.circle.r = 30;
        shape->data.circle.color = RGB(255, 0, 0); // 红色
        shape->selected = FALSE;

        gui->doc.shape_count++;
        select_shape(gui, gui->doc.shape_count - 1);

        char status[256];
        sprintf(status, "添加了圆形 #%d", gui->doc.shape_count);
        update_status(gui, status);
    }
}

// 添加矩形
void add_rect(Win32GUI* gui, int x, int y) {
    if (gui->doc.shape_count < MAX_SHAPES) {
        SvgShape* shape = &gui->doc.shapes[gui->doc.shape_count];
        shape->type = SVG_SHAPE_RECT;
        shape->id = gui->doc.shape_count + 1;
        shape->data.rect.x = x - 40;
        shape->data.rect.y = y - 30;
        shape->data.rect.width = 80;
        shape->data.rect.height = 60;
        shape->data.rect.color = RGB(0, 0, 255); // 蓝色
        shape->selected = FALSE;

        gui->doc.shape_count++;
        select_shape(gui, gui->doc.shape_count - 1);

        char status[256];
        sprintf(status, "添加了矩形 #%d", gui->doc.shape_count);
        update_status(gui, status);
    }
}

// 添加直线
void add_line(Win32GUI* gui, int x, int y) {
    if (gui->doc.shape_count < MAX_SHAPES) {
        SvgShape* shape = &gui->doc.shapes[gui->doc.shape_count];
        shape->type = SVG_SHAPE_LINE;
        shape->id = gui->doc.shape_count + 1;
        shape->data.line.x1 = x - 50;
        shape->data.line.y1 = y;
        shape->data.line.x2 = x + 50;
        shape->data.line.y2 = y;
        shape->data.line.color = RGB(0, 128, 0); // 绿色
        shape->selected = FALSE;

        gui->doc.shape_count++;
        select_shape(gui, gui->doc.shape_count - 1);

        char status[256];
        sprintf(status, "添加了直线 #%d", gui->doc.shape_count);
        update_status(gui, status);
    }
}

// 清空所有图形
void clear_all_shapes(Win32GUI* gui) {
    gui->doc.shape_count = 0;
    gui->doc.selected_shape_id = -1;
    update_status(gui, "已清空所有图形");
}

// 导出SVG文件
void export_svg(Win32GUI* gui) {
    OPENFILENAME ofn;
    char filename[MAX_PATH] = "";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = gui->hwnd;
    ofn.lpstrFilter = "SVG Files\0*.svg\0All Files\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = "svg";

    if (GetSaveFileName(&ofn)) {
        FILE* file = fopen(filename, "w");
        if (file) {
            fprintf(file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
            fprintf(file, "<svg width=\"%.0f\" height=\"%.0f\" xmlns=\"http://www.w3.org/2000/svg\">\n",
                    gui->doc.width, gui->doc.height);

            for (int i = 0; i < gui->doc.shape_count; i++) {
                SvgShape* shape = &gui->doc.shapes[i];

                switch (shape->type) {
                    case SVG_SHAPE_CIRCLE:
                        fprintf(file, "  <circle cx=\"%.1f\" cy=\"%.1f\" r=\"%.1f\" fill=\"#%02X%02X%02X\"/>\n",
                                shape->data.circle.cx, shape->data.circle.cy, shape->data.circle.r,
                                GetRValue(shape->data.circle.color),
                                GetGValue(shape->data.circle.color),
                                GetBValue(shape->data.circle.color));
                        break;
                    case SVG_SHAPE_RECT:
                        fprintf(file, "  <rect x=\"%.1f\" y=\"%.1f\" width=\"%.1f\" height=\"%.1f\" fill=\"#%02X%02X%02X\"/>\n",
                                shape->data.rect.x, shape->data.rect.y,
                                shape->data.rect.width, shape->data.rect.height,
                                GetRValue(shape->data.rect.color),
                                GetGValue(shape->data.rect.color),
                                GetBValue(shape->data.rect.color));
                        break;
                    case SVG_SHAPE_LINE:
                        fprintf(file, "  <line x1=\"%.1f\" y1=\"%.1f\" x2=\"%.1f\" y2=\"%.1f\" stroke=\"#%02X%02X%02X\" stroke-width=\"2\"/>\n",
                                shape->data.line.x1, shape->data.line.y1,
                                shape->data.line.x2, shape->data.line.y2,
                                GetRValue(shape->data.line.color),
                                GetGValue(shape->data.line.color),
                                GetBValue(shape->data.line.color));
                        break;
                }
            }

            fprintf(file, "</svg>\n");
            fclose(file);

            char status[256];
            sprintf(status, "已导出到文件: %s", filename);
            update_status(gui, status);
        } else {
            update_status(gui, "保存文件失败");
        }
    }
}

// 更新状态文本
void update_status(Win32GUI* gui, const char* text) {
    SetWindowText(gui->status_text, text);
}

// 处理画布点击
void handle_canvas_click(Win32GUI* gui, int x, int y) {
    if (gui->current_tool == TOOL_SELECT) {
        clear_selection(gui);

        // 从上到下检查点击的图形
        for (int i = gui->doc.shape_count - 1; i >= 0; i--) {
            SvgShape* shape = &gui->doc.shapes[i];
            BOOL hit = FALSE;

            switch (shape->type) {
                case SVG_SHAPE_CIRCLE:
                    {
                        double dx = x - shape->data.circle.cx;
                        double dy = y - shape->data.circle.cy;
                        if (sqrt(dx*dx + dy*dy) <= shape->data.circle.r) {
                            hit = TRUE;
                        }
                    }
                    break;
                case SVG_SHAPE_RECT:
                    if (x >= shape->data.rect.x &&
                        x <= shape->data.rect.x + shape->data.rect.width &&
                        y >= shape->data.rect.y &&
                        y <= shape->data.rect.y + shape->data.rect.height) {
                        hit = TRUE;
                    }
                    break;
                case SVG_SHAPE_LINE:
                    // 简化的直线点击检测
                    {
                        double x1 = shape->data.line.x1;
                        double y1 = shape->data.line.y1;
                        double x2 = shape->data.line.x2;
                        double y2 = shape->data.line.y2;
                        double len = sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
                        double d1 = sqrt((x-x1)*(x-x1) + (y-y1)*(y-y1));
                        double d2 = sqrt((x-x2)*(x-x2) + (y-y2)*(y-y2));
                        if (d1 + d2 <= len + 5) { // 5像素容差
                            hit = TRUE;
                        }
                    }
                    break;
            }

            if (hit) {
                select_shape(gui, i);
                gui->is_dragging = TRUE;
                gui->drag_start_x = x;
                gui->drag_start_y = y;
                return;
            }
        }

        // 没有点击到任何图形
        clear_selection(gui);
        update_status(gui, "未选中任何图形");
    } else {
        // 添加新图形
        switch (gui->current_tool) {
            case TOOL_CIRCLE:
                add_circle(gui, x, y);
                break;
            case TOOL_RECT:
                add_rect(gui, x, y);
                break;
            case TOOL_LINE:
                add_line(gui, x, y);
                break;
        }
    }
}

// 更新拖拽
void update_dragging(Win32GUI* gui, int x, int y) {
    if (!gui->is_dragging || gui->doc.selected_shape_id < 0) return;

    int dx = x - gui->drag_start_x;
    int dy = y - gui->drag_start_y;

    SvgShape* shape = &gui->doc.shapes[gui->doc.selected_shape_id];

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

    gui->drag_start_x = x;
    gui->drag_start_y = y;

    // 更新状态
    char status[256];
    switch (shape->type) {
        case SVG_SHAPE_CIRCLE:
            sprintf(status, "移动圆形 #%d: 中心(%.1f,%.1f)",
                    shape->id, shape->data.circle.cx, shape->data.circle.cy);
            break;
        case SVG_SHAPE_RECT:
            sprintf(status, "移动矩形 #%d: 位置(%.1f,%.1f)",
                    shape->id, shape->data.rect.x, shape->data.rect.y);
            break;
        case SVG_SHAPE_LINE:
            sprintf(status, "移动直线 #%d",
                    shape->id);
            break;
    }
    update_status(gui, status);
}

// 窗口过程
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Win32GUI* gui = (Win32GUI*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (uMsg) {
        case WM_CREATE:
            gui = (Win32GUI*)((CREATESTRUCT*)lParam)->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)gui);
            gui->hwnd = hwnd;

            // 初始化SVG文档
            gui->doc.width = CANVAS_WIDTH;
            gui->doc.height = CANVAS_HEIGHT;
            gui->doc.shape_count = 0;
            gui->doc.selected_shape_id = -1;
            gui->current_tool = TOOL_SELECT;
            gui->is_dragging = FALSE;

            init_gui_controls(hwnd, gui);
            break;

        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);

                // 创建画布DC
                if (!gui->canvas_dc) {
                    gui->canvas_dc = CreateCompatibleDC(hdc);
                    gui->canvas_bitmap = CreateCompatibleBitmap(hdc, CANVAS_WIDTH, CANVAS_HEIGHT);
                    gui->old_bitmap = SelectObject(gui->canvas_dc, gui->canvas_bitmap);
                }

                // 绘制画布内容
                draw_canvas(gui);

                // 将画布复制到窗口
                BitBlt(hdc, TOOLBAR_WIDTH, 50, CANVAS_WIDTH, CANVAS_HEIGHT,
                        gui->canvas_dc, 0, 0, SRCCOPY);

                // 绘制工具栏背景
                RECT toolbar_rect = {0, 0, TOOLBAR_WIDTH, WINDOW_HEIGHT};
                HBRUSH toolbar_brush = CreateSolidBrush(RGB(230, 230, 230));
                FillRect(hdc, &toolbar_rect, toolbar_brush);
                DeleteObject(toolbar_brush);

                // 绘制画布边框
                HPEN black_pen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
                SelectObject(hdc, black_pen);
                Rectangle(hdc, TOOLBAR_WIDTH, 50, TOOLBAR_WIDTH + CANVAS_WIDTH, 50 + CANVAS_HEIGHT);
                DeleteObject(black_pen);

                // 绘制标题
                SetBkMode(hdc, TRANSPARENT);
                HFONT title_font = CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                           DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Microsoft YaHei");
                HFONT old_font = SelectObject(hdc, title_font);
                TextOut(hdc, TOOLBAR_WIDTH + 10, 10, "SVG 图形编辑器", 14);
                SelectObject(hdc, old_font);
                DeleteObject(title_font);

                EndPaint(hwnd, &ps);
            }
            return 0;

        case WM_COMMAND:
            {
                int wmId = LOWORD(wParam);
                switch (wmId) {
                    case ID_BUTTON_SELECT:
                        gui->current_tool = TOOL_SELECT;
                        update_status(gui, "选择工具 - 点击图形进行选择和移动");
                        break;
                    case ID_BUTTON_CIRCLE:
                        gui->current_tool = TOOL_CIRCLE;
                        update_status(gui, "圆形工具 - 点击画布添加圆形");
                        break;
                    case ID_BUTTON_RECT:
                        gui->current_tool = TOOL_RECT;
                        update_status(gui, "矩形工具 - 点击画布添加矩形");
                        break;
                    case ID_BUTTON_LINE:
                        gui->current_tool = TOOL_LINE;
                        update_status(gui, "直线工具 - 点击画布添加直线");
                        break;
                    case ID_BUTTON_CLEAR:
                        if (MessageBox(hwnd, "确定要清空所有图形吗？", "确认", MB_OKCANCEL) == IDOK) {
                            clear_all_shapes(gui);
                        }
                        break;
                    case ID_BUTTON_EXPORT:
                        export_svg(gui);
                        break;
                    case ID_BUTTON_EXIT:
                        DestroyWindow(hwnd);
                        break;
                }
            }
            return 0;

        case WM_LBUTTONDOWN:
            {
                int x = GET_X_LPARAM(lParam) - TOOLBAR_WIDTH;
                int y = GET_Y_LPARAM(lParam) - 50;

                if (x >= 0 && x < CANVAS_WIDTH && y >= 0 && y < CANVAS_HEIGHT) {
                    handle_canvas_click(gui, x, y);
                    SetCapture(hwnd);
                }
            }
            break;

        case WM_MOUSEMOVE:
            {
                int x = GET_X_LPARAM(lParam) - TOOLBAR_WIDTH;
                int y = GET_Y_LPARAM(lParam) - 50;

                if (x >= 0 && x < CANVAS_WIDTH && y >= 0 && y < CANVAS_HEIGHT) {
                    if (wParam & MK_LBUTTON) {
                        update_dragging(gui, x, y);
                    }

                    // 更新鼠标位置
                    gui->last_mouse_pos.x = x;
                    gui->last_mouse_pos.y = y;
                }
            }
            break;

        case WM_LBUTTONUP:
            if (gui->is_dragging) {
                gui->is_dragging = FALSE;
                ReleaseCapture();
            }
            break;

        case WM_DESTROY:
            if (gui->canvas_dc) {
                SelectObject(gui->canvas_dc, gui->old_bitmap);
                DeleteObject(gui->canvas_bitmap);
                DeleteObject(gui->canvas_dc);
            }
            PostQuitMessage(0);
            return 0;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// 主函数
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 注册窗口类
    const char* CLASS_NAME = "SVGEditorWindowClass";

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClass(&wc);

    // 创建GUI实例
    Win32GUI gui = {0};

    // 创建窗口
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "SVG 图形编辑器",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, hInstance, &gui
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // 消息循环
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}