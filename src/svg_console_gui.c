#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <windows.h>
#include <conio.h>

#define MAX_SHAPES 100
#define CANVAS_WIDTH 60
#define CANVAS_HEIGHT 25
#define COLOR_RED 12
#define COLOR_GREEN 10
#define COLOR_BLUE 9
#define COLOR_YELLOW 14
#define COLOR_CYAN 11
#define COLOR_WHITE 15
#define COLOR_GRAY 8

// 图形结构
typedef enum {
    SVG_SHAPE_CIRCLE,
    SVG_SHAPE_RECT,
    SVG_SHAPE_LINE
} SvgShapeType;

typedef struct {
    double cx, cy, r;
    int color;
    char color_str[16];
} SvgCircle;

typedef struct {
    double x, y, width, height;
    int color;
    char color_str[16];
} SvgRect;

typedef struct {
    double x1, y1, x2, y2;
    int color;
    char color_str[16];
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
    SvgShape shapes[MAX_SHAPES];
    int shape_count;
    int selected_id;
    char canvas[CANVAS_HEIGHT][CANVAS_WIDTH + 1];
} ConsoleGUI;

// 控制台函数
void set_color(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void clear_screen() {
    system("cls");
}

void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void hide_cursor() {
    CONSOLE_CURSOR_INFO cursor_info;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
    cursor_info.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
}

void show_cursor() {
    CONSOLE_CURSOR_INFO cursor_info;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
    cursor_info.bVisible = TRUE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
}

// 解析颜色
int parse_color(const char* color_str) {
    if (!color_str) return COLOR_WHITE;

    if (strstr(color_str, "red")) return COLOR_RED;
    if (strstr(color_str, "green")) return COLOR_GREEN;
    if (strstr(color_str, "blue")) return COLOR_BLUE;
    if (strstr(color_str, "yellow")) return COLOR_YELLOW;
    if (strstr(color_str, "cyan")) return COLOR_CYAN;
    if (strstr(color_str, "white")) return COLOR_WHITE;
    if (strstr(color_str, "gray")) return COLOR_GRAY;

    return COLOR_WHITE;
}

// 初始化画布
void init_canvas(ConsoleGUI* gui) {
    for (int y = 0; y < CANVAS_HEIGHT; y++) {
        for (int x = 0; x < CANVAS_WIDTH; x++) {
            gui->canvas[y][x] = ' ';
        }
        gui->canvas[y][CANVAS_WIDTH] = '\0';
    }
}

// 绘制图形到画布
void draw_shape_to_canvas(ConsoleGUI* gui, SvgShape* shape) {
    // 将图形坐标映射到控制台坐标
    int scale_x = 3;  // 每个控制台字符代表3个像素单位
    int scale_y = 1;  // 每个控制台行代表1个像素单位

    switch (shape->type) {
        case SVG_SHAPE_CIRCLE:
            {
                int cx = (int)(shape->data.circle.cx / scale_x);
                int cy = (int)(shape->data.circle.cy / scale_y);
                int r = (int)(shape->data.circle.r / scale_x);
                char c = shape->data.circle.color_str[0];
                if (c == '#') c = 'O';

                for (int y = -r; y <= r; y++) {
                    for (int x = -r; x <= r; x++) {
                        if (x*x + y*y <= r*r) {
                            int px = cx + x;
                            int py = cy + y;
                            if (px >= 0 && px < CANVAS_WIDTH && py >= 0 && py < CANVAS_HEIGHT) {
                                gui->canvas[py][px] = c;
                            }
                        }
                    }
                }
            }
            break;

        case SVG_SHAPE_RECT:
            {
                int x1 = (int)(shape->data.rect.x / scale_x);
                int y1 = (int)(shape->data.rect.y / scale_y);
                int w = (int)(shape->data.rect.width / scale_x);
                int h = (int)(shape->data.rect.height / scale_y);
                char c = shape->data.rect.color_str[0];
                if (c == '#') c = 'X';

                for (int y = y1; y < y1 + h && y < CANVAS_HEIGHT; y++) {
                    for (int x = x1; x < x1 + w && x < CANVAS_WIDTH; x++) {
                        if (x >= 0 && y >= 0) {
                            gui->canvas[y][x] = c;
                        }
                    }
                }
            }
            break;

        case SVG_SHAPE_LINE:
            {
                int x1 = (int)(shape->data.line.x1 / scale_x);
                int y1 = (int)(shape->data.line.y1 / scale_y);
                int x2 = (int)(shape->data.line.x2 / scale_x);
                int y2 = (int)(shape->data.line.y2 / scale_y);
                char c = shape->data.line.color_str[0];
                if (c == '#') c = '*';

                // Bresenham直线算法
                int dx = abs(x2 - x1);
                int dy = abs(y2 - y1);
                int sx = x1 < x2 ? 1 : -1;
                int sy = y1 < y2 ? 1 : -1;
                int err = dx - dy;

                while (1) {
                    if (x1 >= 0 && x1 < CANVAS_WIDTH && y1 >= 0 && y1 < CANVAS_HEIGHT) {
                        gui->canvas[y1][x1] = c;
                    }

                    if (x1 == x2 && y1 == y2) break;

                    int e2 = 2 * err;
                    if (e2 > -dy) {
                        err -= dy;
                        x1 += sx;
                    }
                    if (e2 < dx) {
                        err += dx;
                        y1 += sy;
                    }
                }
            }
            break;
    }

    // 绘制选中框
    if (shape->id == gui->selected_id) {
        set_color(COLOR_RED);
        switch (shape->type) {
            case SVG_SHAPE_CIRCLE:
                {
                    int cx = (int)(shape->data.circle.cx / 3);
                    int cy = (int)(shape->data.circle.cy);
                    int r = (int)(shape->data.circle.r / 3);

                    for (int angle = 0; angle < 360; angle += 45) {
                        double rad = angle * 3.14159 / 180.0;
                        int x = cx + (int)(r * cos(rad));
                        int y = cy + (int)(r * sin(rad));
                        if (x >= 0 && x < CANVAS_WIDTH && y >= 0 && y < CANVAS_HEIGHT) {
                            gui->canvas[y][x] = '#';
                        }
                    }
                }
                break;
            case SVG_SHAPE_RECT:
                {
                    int x1 = (int)(shape->data.rect.x / 3) - 1;
                    int y1 = (int)(shape->data.rect.y) - 1;
                    int w = (int)(shape->data.rect.width / 3) + 2;
                    int h = (int)(shape->data.rect.height) + 2;

                    for (int x = x1; x < x1 + w && x < CANVAS_WIDTH; x++) {
                        if (x >= 0 && y1 >= 0 && y1 < CANVAS_HEIGHT) gui->canvas[y1][x] = '#';
                        if (x >= 0 && y1 + h - 1 >= 0 && y1 + h - 1 < CANVAS_HEIGHT) {
                            gui->canvas[y1 + h - 1][x] = '#';
                        }
                    }
                    for (int y = y1; y < y1 + h && y < CANVAS_HEIGHT; y++) {
                        if (x1 >= 0 && y >= 0) gui->canvas[y][x1] = '#';
                        if (x1 + w - 1 >= 0 && y >= 0 && x1 + w - 1 < CANVAS_WIDTH) {
                            gui->canvas[y][x1 + w - 1] = '#';
                        }
                    }
                }
                break;
        }
    }
}

// 显示画布
void display_canvas(ConsoleGUI* gui) {
    gotoxy(5, 3);

    // 绘制边框
    printf("┌");
    for (int x = 0; x < CANVAS_WIDTH; x++) printf("─");
    printf("┐\n");

    for (int y = 0; y < CANVAS_HEIGHT; y++) {
        gotoxy(5, 4 + y);
        printf("│");

        for (int x = 0; x < CANVAS_WIDTH; x++) {
            // 根据字符类型设置颜色
            char c = gui->canvas[y][x];
            if (c == 'O' || c == 'o') set_color(COLOR_RED);
            else if (c == 'X' || c == 'x') set_color(COLOR_BLUE);
            else if (c == '*' || c == '+') set_color(COLOR_GREEN);
            else if (c == '#') set_color(COLOR_RED);
            else set_color(COLOR_WHITE);

            printf("%c", c);
        }

        set_color(COLOR_WHITE);
        printf("│\n");
    }

    gotoxy(5, 4 + CANVAS_HEIGHT);
    printf("└");
    for (int x = 0; x < CANVAS_WIDTH; x++) printf("─");
    printf("┘");
}

// 显示菜单
void display_menu(ConsoleGUI* gui) {
    gotoxy(2, 0);
    set_color(COLOR_CYAN);
    printf("╔════════════════════════════════════════════════════════╗");
    gotoxy(2, 1);
    printf("║                  SVG 控制台图形编辑器                  ║");
    gotoxy(2, 2);
    printf("╚════════════════════════════════════════════════════════╝");

    gotoxy(75, 4);
    set_color(COLOR_YELLOW);
    printf("工具菜单:");

    gotoxy(75, 6);
    set_color(COLOR_GREEN);
    printf("1. 选择工具");

    gotoxy(75, 7);
    set_color(COLOR_RED);
    printf("2. 圆形工具");

    gotoxy(75, 8);
    set_color(COLOR_BLUE);
    printf("3. 矩形工具");

    gotoxy(75, 9);
    set_color(COLOR_CYAN);
    printf("4. 直线工具");

    gotoxy(75, 11);
    set_color(COLOR_GRAY);
    printf("5. 清空画布");

    gotoxy(75, 12);
    set_color(COLOR_WHITE);
    printf("6. 保存SVG");

    gotoxy(75, 13);
    set_color(COLOR_WHITE);
    printf("Q. 退出");

    gotoxy(75, 16);
    set_color(COLOR_YELLOW);
    printf("当前图形数: %d", gui->shape_count);

    if (gui->selected_id >= 0) {
        gotoxy(75, 17);
        set_color(COLOR_RED);
        printf("已选中: #%d", gui->selected_id);
    }

    gotoxy(75, 20);
    set_color(COLOR_GRAY);
    printf("使用鼠标点击或");
    gotoxy(75, 21);
    printf("按键 1-4 切换工具");
}

// 显示图形列表
void display_shapes(ConsoleGUI* gui) {
    gotoxy(5, 35);
    set_color(COLOR_CYAN);
    printf("图形列表:");

    for (int i = 0; i < gui->shape_count && i < 10; i++) {
        SvgShape* shape = &gui->shapes[i];
        gotoxy(5, 36 + i);

        if (shape->id == gui->selected_id) {
            set_color(COLOR_RED);
            printf("▶ ");
        } else {
            set_color(COLOR_GRAY);
            printf("  ");
        }

        switch (shape->type) {
            case SVG_SHAPE_CIRCLE:
                set_color(COLOR_RED);
                printf("圆形#%d: 中心(%.0f,%.0f) r=%.0f",
                       shape->id, shape->data.circle.cx, shape->data.circle.cy, shape->data.circle.r);
                break;
            case SVG_SHAPE_RECT:
                set_color(COLOR_BLUE);
                printf("矩形#%d: 位置(%.0f,%.0f) 大小%.0fx%.0f",
                       shape->id, shape->data.rect.x, shape->data.rect.y,
                       shape->data.rect.width, shape->data.rect.height);
                break;
            case SVG_SHAPE_LINE:
                set_color(COLOR_GREEN);
                printf("直线#%d: (%.0f,%.0f)-(%.0f,%.0f)",
                       shape->id, shape->data.line.x1, shape->data.line.y1,
                       shape->data.line.x2, shape->data.line.y2);
                break;
        }
    }
}

// 添加图形
void add_circle(ConsoleGUI* gui, double cx, double cy, double r, const char* color) {
    if (gui->shape_count < MAX_SHAPES) {
        SvgShape* shape = &gui->shapes[gui->shape_count];
        shape->type = SVG_SHAPE_CIRCLE;
        shape->id = gui->shape_count + 1;
        shape->data.circle.cx = cx;
        shape->data.circle.cy = cy;
        shape->data.circle.r = r;
        shape->data.circle.color = parse_color(color);
        strcpy(shape->data.circle.color_str, color);

        gui->shape_count++;
        gui->selected_id = shape->id;
    }
}

void add_rect(ConsoleGUI* gui, double x, double y, double w, double h, const char* color) {
    if (gui->shape_count < MAX_SHAPES) {
        SvgShape* shape = &gui->shapes[gui->shape_count];
        shape->type = SVG_SHAPE_RECT;
        shape->id = gui->shape_count + 1;
        shape->data.rect.x = x;
        shape->data.rect.y = y;
        shape->data.rect.width = w;
        shape->data.rect.height = h;
        shape->data.rect.color = parse_color(color);
        strcpy(shape->data.rect.color_str, color);

        gui->shape_count++;
        gui->selected_id = shape->id;
    }
}

void add_line(ConsoleGUI* gui, double x1, double y1, double x2, double y2, const char* color) {
    if (gui->shape_count < MAX_SHAPES) {
        SvgShape* shape = &gui->shapes[gui->shape_count];
        shape->type = SVG_SHAPE_LINE;
        shape->id = gui->shape_count + 1;
        shape->data.line.x1 = x1;
        shape->data.line.y1 = y1;
        shape->data.line.x2 = x2;
        shape->data.line.y2 = y2;
        shape->data.line.color = parse_color(color);
        strcpy(shape->data.line.color_str, color);

        gui->shape_count++;
        gui->selected_id = shape->id;
    }
}

// 保存SVG文件
void save_svg(ConsoleGUI* gui, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        gotoxy(5, 48);
        set_color(COLOR_RED);
        printf("无法保存文件: %s", filename);
        return;
    }

    fprintf(file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(file, "<svg width=\"180\" height=\"25\" xmlns=\"http://www.w3.org/2000/svg\">\n");

    for (int i = 0; i < gui->shape_count; i++) {
        SvgShape* shape = &gui->shapes[i];

        switch (shape->type) {
            case SVG_SHAPE_CIRCLE:
                fprintf(file, "  <circle cx=\"%.1f\" cy=\"%.1f\" r=\"%.1f\" fill=\"%s\"/>\n",
                        shape->data.circle.cx * 3, shape->data.circle.cy, shape->data.circle.r * 3,
                        shape->data.circle.color_str);
                break;
            case SVG_SHAPE_RECT:
                fprintf(file, "  <rect x=\"%.1f\" y=\"%.1f\" width=\"%.1f\" height=\"%.1f\" fill=\"%s\"/>\n",
                        shape->data.rect.x * 3, shape->data.rect.y,
                        shape->data.rect.width * 3, shape->data.rect.height,
                        shape->data.rect.color_str);
                break;
            case SVG_SHAPE_LINE:
                fprintf(file, "  <line x1=\"%.1f\" y1=\"%.1f\" x2=\"%.1f\" y2=\"%.1f\" stroke=\"%s\" stroke-width=\"2\"/>\n",
                        shape->data.line.x1 * 3, shape->data.line.y1,
                        shape->data.line.x2 * 3, shape->data.line.y2,
                        shape->data.line.color_str);
                break;
        }
    }

    fprintf(file, "</svg>\n");
    fclose(file);

    gotoxy(5, 48);
    set_color(COLOR_GREEN);
    printf("已保存到: %s", filename);
}

// 清空图形
void clear_shapes(ConsoleGUI* gui) {
    gui->shape_count = 0;
    gui->selected_id = -1;
}

// 主循环
int main() {
    ConsoleGUI gui = {0};
    gui.selected_id = -1;

    // 设置控制台
    system("chcp 65001 >nul");
    clear_screen();
    hide_cursor();

    printf("初始化控制台SVG编辑器...\n");
    Sleep(500);

    int tool = 1; // 1=选择, 2=圆形, 3=矩形, 4=直线
    int running = 1;

    while (running) {
        init_canvas(&gui);

        // 重绘所有图形
        for (int i = 0; i < gui.shape_count; i++) {
            draw_shape_to_canvas(&gui, &gui.shapes[i]);
        }

        clear_screen();
        display_canvas(&gui);
        display_menu(&gui);
        display_shapes(&gui);

        gotoxy(75, 23);
        set_color(COLOR_YELLOW);
        printf("当前工具: ");
        switch (tool) {
            case 1: printf("选择"); break;
            case 2: printf("圆形"); break;
            case 3: printf("矩形"); break;
            case 4: printf("直线"); break;
        }

        // 输入提示
        gotoxy(5, 48);
        set_color(COLOR_WHITE);
        printf("按 1-4 切换工具 | 按数字键选择图形 | 按 5 清空 | 按 6 保存 | 按 Q 退出");

        // 检查按键
        if (_kbhit()) {
            char key = _getch();

            switch (key) {
                case '1': tool = 1; break;
                case '2': tool = 2; break;
                case '3': tool = 3; break;
                case '4': tool = 4; break;
                case '5':
                    clear_shapes(&gui);
                    break;
                case '6':
                    save_svg(&gui, "output.svg");
                    Sleep(1000);
                    break;
                case 'q':
                case 'Q':
                    running = 0;
                    break;
                case 's':
                case 'S':
                    if (tool == 1 && gui.shape_count > 0) {
                        gui.selected_id = (gui.selected_id + 1) % (gui.shape_count + 1);
                    }
                    break;
                case 'c':
                case 'C':
                    if (tool == 2) add_circle(&gui, 90, 12, 8, "red");
                    else if (tool == 3) add_rect(&gui, 75, 10, 30, 8, "blue");
                    else if (tool == 4) add_line(&gui, 60, 12, 120, 12, "green");
                    break;
                case 'r':
                case 'R':
                    if (gui.selected_id >= 1 && gui.selected_id <= gui.shape_count) {
                        // 简单的右移
                        SvgShape* shape = &gui.shapes[gui.selected_id - 1];
                        switch (shape->type) {
                            case SVG_SHAPE_CIRCLE:
                                shape->data.circle.cx += 3;
                                break;
                            case SVG_SHAPE_RECT:
                                shape->data.rect.x += 3;
                                break;
                            case SVG_SHAPE_LINE:
                                shape->data.line.x1 += 3;
                                shape->data.line.x2 += 3;
                                break;
                        }
                    }
                    break;
                case 'l':
                case 'L':
                    if (gui.selected_id >= 1 && gui.selected_id <= gui.shape_count) {
                        // 简单的左移
                        SvgShape* shape = &gui.shapes[gui.selected_id - 1];
                        switch (shape->type) {
                            case SVG_SHAPE_CIRCLE:
                                shape->data.circle.cx -= 3;
                                break;
                            case SVG_SHAPE_RECT:
                                shape->data.rect.x -= 3;
                                break;
                            case SVG_SHAPE_LINE:
                                shape->data.line.x1 -= 3;
                                shape->data.line.x2 -= 3;
                                break;
                        }
                    }
                    break;
                case 'd':
                case 'D':
                    if (gui.selected_id >= 1 && gui.selected_id <= gui.shape_count) {
                        // 删除选中的图形
                        int index = gui.selected_id - 1;
                        for (int i = index; i < gui.shape_count - 1; i++) {
                            gui.shapes[i] = gui.shapes[i + 1];
                            gui.shapes[i].id--;
                        }
                        gui.shape_count--;
                        gui.selected_id = -1;
                    }
                    break;
            }
        }

        Sleep(50); // 减少CPU使用
    }

    show_cursor();
    clear_screen();
    set_color(COLOR_WHITE);
    printf("感谢使用SVG控制台编辑器！\n");

    return 0;
}