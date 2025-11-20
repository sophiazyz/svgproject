#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_SHAPES 100
#define MAX_LINE_LENGTH 256

typedef enum {
    SVG_SHAPE_CIRCLE,
    SVG_SHAPE_RECT,
    SVG_SHAPE_LINE
} SvgShapeType;

typedef struct {
    double cx, cy, r;
    char fill[16];
} SvgCircle;

typedef struct {
    double x, y, width, height;
    char fill[16];
} SvgRect;

typedef struct {
    double x1, y1, x2, y2;
    char stroke[16];
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
    int selected_shape;
} SvgDocument;

static void read_full_tag(FILE *fp, const char *first_line, char *out_tag)
{
    strcpy(out_tag, first_line);

    while (!strchr(out_tag, '>')) {
        char buf[2048];
        if (!fgets(buf, sizeof(buf), fp)) break;
        strcat(out_tag, buf);
    }
}
// Parse color from string
void gui_parse_color(const char* color_str, char* output) {
    if (strncmp(color_str, "#", 1) == 0) {
        strncpy(output, color_str, 15);
        output[15] = '\0';
    } else if (strstr(color_str, "red")) {
        strcpy(output, "#FF0000");
    } else if (strstr(color_str, "green")) {
        strcpy(output, "#00FF00");
    } else if (strstr(color_str, "blue")) {
        strcpy(output, "#0000FF");
    } else if (strstr(color_str, "yellow")) {
        strcpy(output, "#FFFF00");
    } else if (strstr(color_str, "orange")) {
        strcpy(output, "#FFA500");
    } else if (strstr(color_str, "purple")) {
        strcpy(output, "#800080");
    } else if (strstr(color_str, "pink")) {
        strcpy(output, "#FFC0CB");
    } else {
        strcpy(output, "#000000"); // default black
    }
}

// Parse simple SVG file (very basic parser)
int parse_svg_file(const char* filename, SvgDocument* doc) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Cannot open file %s\n", filename);
        return 0;
    }

    char line[MAX_LINE_LENGTH];
    doc->shape_count = 0;
    doc->width = 800; // default
    doc->height = 600; // default

    while (fgets(line, sizeof(line), file)) {
        // Remove whitespace
        char* trimmed = line;
        while (isspace(*trimmed)) trimmed++;
        char full_tag[2048];
        read_full_tag(file, trimmed, full_tag);
        trimmed = full_tag;

        // Parse SVG dimensions
        if (strstr(trimmed, "<svg")) {
            char* width_pos = strstr(trimmed, "width=\"");
            if (width_pos) sscanf(width_pos, "width=\"%lf\"", &doc->width);
            
            char* height_pos = strstr(trimmed, "height=\"");
            if (height_pos) sscanf(height_pos, "height=\"%lf\"", &doc->height);
        }

        // Parse circle
        else if (strstr(trimmed, "<circle")) {
            if (doc->shape_count >= MAX_SHAPES) break;
            
            SvgShape* shape = &doc->shapes[doc->shape_count];
            shape->type = SVG_SHAPE_CIRCLE;
            shape->id = doc->shape_count + 1;
            
            sscanf(trimmed, "<circle cx=\"%lf\" cy=\"%lf\" r=\"%lf\"", 
                   &shape->data.circle.cx, &shape->data.circle.cy, &shape->data.circle.r);
            
            char* fill_pos = strstr(trimmed, "fill=\"");
            if (fill_pos) {
                char color[32];
                sscanf(fill_pos, "fill=\"%31[^\"]\"", color);
                gui_parse_color(color, shape->data.circle.fill);
            } else {
                strcpy(shape->data.circle.fill, "#000000");
            }
            
            doc->shape_count++;
        }

        // Parse rectangle
        else if (strstr(trimmed, "<rect")) {
            if (doc->shape_count >= MAX_SHAPES) break;
            
            SvgShape* shape = &doc->shapes[doc->shape_count];
            shape->type = SVG_SHAPE_RECT;
            shape->id = doc->shape_count + 1;
            
            sscanf(trimmed, "<rect x=\"%lf\" y=\"%lf\" width=\"%lf\" height=\"%lf\"", 
                   &shape->data.rect.x, &shape->data.rect.y, 
                   &shape->data.rect.width, &shape->data.rect.height);
            
            char* fill_pos = strstr(trimmed, "fill=\"");
            if (fill_pos) {
                char color[32];
                sscanf(fill_pos, "fill=\"%31[^\"]\"", color);
                gui_parse_color(color, shape->data.rect.fill);
            } else {
                strcpy(shape->data.rect.fill, "#000000");
            }
            
            doc->shape_count++;
        }

        // Parse line
        else if (strstr(trimmed, "<line")) {
            if (doc->shape_count >= MAX_SHAPES) break;
            
            SvgShape* shape = &doc->shapes[doc->shape_count];
            shape->type = SVG_SHAPE_LINE;
            shape->id = doc->shape_count + 1;
            
            sscanf(trimmed, "<line x1=\"%lf\" y1=\"%lf\" x2=\"%lf\" y2=\"%lf\"", 
                   &shape->data.line.x1, &shape->data.line.y1,
                   &shape->data.line.x2, &shape->data.line.y2);
            
            char* stroke_pos = strstr(trimmed, "stroke=\"");
            if (stroke_pos) {
                char color[32];
                sscanf(stroke_pos, "stroke=\"%31[^\"]\"", color);
                gui_parse_color(color, shape->data.line.stroke);
            } else {
                strcpy(shape->data.line.stroke, "#000000");
            }
            
            doc->shape_count++;
        }
    }

    fclose(file);
    return 1;
}

// Display SVG summary
void display_summary(SvgDocument* doc) {
    printf("\n=== SVG Document Summary ===\n");
    printf("Canvas: %.1f x %.1f\n", doc->width, doc->height);
    printf("Total shapes: %d\n\n", doc->shape_count);
}

// Display all shapes
void display_shapes(SvgDocument* doc) {
    printf("=== Shapes ===\n");
    for (int i = 0; i < doc->shape_count; i++) {
        SvgShape* shape = &doc->shapes[i];
        printf("[%d] ", shape->id);
        
        switch (shape->type) {
            case SVG_SHAPE_CIRCLE:
                printf("CIRCLE: center=(%.1f,%.1f) radius=%.1f fill=%s\n", 
                       shape->data.circle.cx, shape->data.circle.cy, 
                       shape->data.circle.r, shape->data.circle.fill);
                break;
                
            case SVG_SHAPE_RECT:
                printf("RECT: position=(%.1f,%.1f) size=%.1fx%.1f fill=%s\n", 
                       shape->data.rect.x, shape->data.rect.y,
                       shape->data.rect.width, shape->data.rect.height,
                       shape->data.rect.fill);
                break;
                
            case SVG_SHAPE_LINE:
                printf("LINE: from (%.1f,%.1f) to (%.1f,%.1f) stroke=%s\n", 
                       shape->data.line.x1, shape->data.line.y1,
                       shape->data.line.x2, shape->data.line.y2,
                       shape->data.line.stroke);
                break;
        }
    }
    printf("\n");
}

// Add a new circle
void add_circle(SvgDocument* doc, double cx, double cy, double r, const char* fill) {
    if (doc->shape_count >= MAX_SHAPES) {
        printf("Error: Maximum shapes reached!\n");
        return;
    }
    
    SvgShape* shape = &doc->shapes[doc->shape_count];
    shape->type = SVG_SHAPE_CIRCLE;
    shape->id = doc->shape_count + 1;
    //test
    printf("Adding circle with ID %d\n", shape->id);
    printf("%d\n",doc->shape_count);
    //
    shape->data.circle.cx = cx;
    shape->data.circle.cy = cy;
    shape->data.circle.r = r;
    gui_parse_color(fill, shape->data.circle.fill);
    doc->shape_count++;
    //test
    printf("Adding circle with ID %d\n", shape->id);
    printf("%d\n",doc->shape_count);
    //
    printf("Circle added with ID %d\n", shape->id);
}

// Add a new rectangle
void add_rect(SvgDocument* doc, double x, double y, double w, double h, const char* fill) {
    if (doc->shape_count >= MAX_SHAPES) {
        printf("Error: Maximum shapes reached!\n");
        return;
    }
    
    SvgShape* shape = &doc->shapes[doc->shape_count];
    shape->type = SVG_SHAPE_RECT;
    shape->id = doc->shape_count + 1;
    
    shape->data.rect.x = x;
    shape->data.rect.y = y;
    shape->data.rect.width = w;
    shape->data.rect.height = h;
    gui_parse_color(fill, shape->data.rect.fill);
    
    doc->shape_count++;
    printf("Rectangle added with ID %d\n", shape->id);
}

// Select a shape for editing
void select_shape(SvgDocument* doc, int id) {
    if (id < 1 || id > doc->shape_count) {
        printf("Error: Invalid shape ID\n");
        return;
    }
    
    doc->selected_shape = id - 1;
    printf("Selected shape [%d]\n", id);
}

// Move selected shape
void move_shape(SvgDocument* doc, double dx, double dy) {
    if (doc->selected_shape == -1) {
        printf("Error: No shape selected. Use 'select <id>' first.\n");
        return;
    }
    
    SvgShape* shape = &doc->shapes[doc->selected_shape];
    
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
    
    printf("Moved shape [%d] by (%.1f, %.1f)\n", doc->selected_shape + 1, dx, dy);
}

// Delete selected shape
void delete_shape(SvgDocument* doc) {
    if (doc->selected_shape == -1) {
        printf("Error: No shape selected.\n");
        return;
    }
    
    int index = doc->selected_shape;
    printf("Deleted shape [%d]\n", index + 1);
    
    // Shift all shapes after the deleted one
    for (int i = index; i < doc->shape_count - 1; i++) {
        doc->shapes[i] = doc->shapes[i + 1];
        doc->shapes[i].id = i + 1;
    }
    
    doc->shape_count--;
    doc->selected_shape = -1;
}

// Show help
void show_help() {
    printf("\n=== SVG Editor Commands ===\n");
    printf("load <file.svg>      - Load SVG file\n");
    printf("save <file.svg>      - Save to SVG file\n");
    printf("list                 - Show all shapes\n");
    printf("summary              - Show document summary\n");
    printf("select <id>          - Select shape for editing\n");
    printf("move <dx> <dy>       - Move selected shape\n");
    printf("delete               - Delete selected shape\n");
    printf("add_circle <cx> <cy> <r> [color] - Add circle\n");
    printf("add_rect <x> <y> <w> <h> [color] - Add rectangle\n");
    printf("help                 - Show this help\n");
    printf("quit                 - Exit program\n\n");
}

// Save svg
int save_svg_file(const char* filename, SvgDocument* doc) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Error: Cannot create file %s\n", filename);
        return 0;
    }

    // Write SVG header
    fprintf(file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(file, "<svg width=\"%.1f\" height=\"%.1f\" xmlns=\"http://www.w3.org/2000/svg\">\n", 
            doc->width, doc->height);

    // Write shapes
    for (int i = 0; i < doc->shape_count; i++) {
        SvgShape* shape = &doc->shapes[i];
        
        switch (shape->type) {
            case SVG_SHAPE_CIRCLE: {
                SvgCircle* circle = &shape->data.circle;
                fprintf(file, "  <circle cx=\"%.1f\" cy=\"%.1f\" r=\"%.1f\" fill=\"%s\"/>\n",
                        circle->cx, circle->cy, circle->r, circle->fill);
                break;
            }
                
            case SVG_SHAPE_RECT: {
                SvgRect* rect = &shape->data.rect;
                fprintf(file, "  <rect x=\"%.1f\" y=\"%.1f\" width=\"%.1f\" height=\"%.1f\" fill=\"%s\"/>\n",
                        rect->x, rect->y, rect->width, rect->height, rect->fill);
                break;
            }
                
            case SVG_SHAPE_LINE: {
                SvgLine* line = &shape->data.line;
                fprintf(file, "  <line x1=\"%.1f\" y1=\"%.1f\" x2=\"%.1f\" y2=\"%.1f\" stroke=\"%s\"/>\n",
                        line->x1, line->y1, line->x2, line->y2, line->stroke);
                break;
            }
        }
    }

    // Write SVG footer
    fprintf(file, "</svg>\n");
    fclose(file);
    
    printf("Saved %d shapes to %s\n", doc->shape_count, filename);
    return 1;
}

// Main interactive loop
int main() {
    SvgDocument doc = {0};
    doc.selected_shape = -1;
    
    printf("=== Simple SVG Editor ===\n");
    printf("Type 'help' for commands\n\n");
    
    char command[64];
    char filename[256];
    
    while (1) {
        printf("> ");
        if (scanf("%63s", command) != 1) break;
        
        if (strcmp(command, "load") == 0) {
            scanf("%255s", filename);
            if (parse_svg_file(filename, &doc)) {
                printf("Loaded %s successfully\n", filename);
                display_summary(&doc);
            }
            
        } else if (strcmp(command, "list") == 0) {
            display_shapes(&doc);
            
        } else if (strcmp(command, "summary") == 0) {
            display_summary(&doc);
            
        } else if (strcmp(command, "select") == 0) {
            int id;
            scanf("%d", &id);
            select_shape(&doc, id);
            
        } else if (strcmp(command, "move") == 0) {
            double dx, dy;
            scanf("%lf %lf", &dx, &dy);
            move_shape(&doc, dx, dy);
            
        } else if (strcmp(command, "delete") == 0) {
            delete_shape(&doc);
            
        } else if (strcmp(command, "add_circle") == 0) {
            double cx, cy, r;
            char color[16] = "#000000";
            scanf("%lf %lf %lf", &cx, &cy, &r);
            // Optional color
            if (scanf("%15s", color) == 1) {
                add_circle(&doc, cx, cy, r, color);
            } else {
                add_circle(&doc, cx, cy, r, "#000000");
            }
            
        } else if (strcmp(command, "add_rect") == 0) {
            double x, y, w, h;
            char color[16] = "#000000";
            scanf("%lf %lf %lf %lf", &x, &y, &w, &h);
            // Optional color
            if (scanf("%15s", color) == 1) {
                add_rect(&doc, x, y, w, h, color);
            } else {
                add_rect(&doc, x, y, w, h, "#000000");
            }
            
        } else if (strcmp(command, "help") == 0) {
            show_help();
            
        } else if (strcmp(command, "quit") == 0) {
            printf("Goodbye!\n");
            break;
        
        } else if (strcmp(command, "save") == 0) {
            scanf("%255s", filename);
            if (save_svg_file(filename, &doc)) {
                printf("Saved to %s successfully\n", filename);
            } else {
                printf("Error saving to %s\n", filename);
            }
        } else {
            printf("Unknown command. Type 'help' for available commands.\n");
        }
        
        // Clear input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    }
    
    return 0;
}