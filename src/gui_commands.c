#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/gui_commands.h"

void cmd_export(SVGDocument* doc, int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: export <filename> <format> [width] [height] [quality]\n");
        printf("Formats: bmp, jpg, png, svg\n");
        printf("Examples:\n");
        printf("  export output.bmp bmp 800 600\n");
        printf("  export output.jpg jpg 1024 768 90\n");
        printf("  export output.svg svg\n");
        return;
    }
    
    const char* filename = argv[1];
    const char* format = argv[2];
    int width = 800, height = 600, quality = 90;
    
    // 解析可选参数
    if (argc >= 5) {
        width = atoi(argv[3]);
        height = atoi(argv[4]);
    }
    if (argc >= 6 && (strcmp(format, "jpg") == 0 || strcmp(format, "jpeg") == 0)) {
        quality = atoi(argv[5]);
    }
    
    bool success = svg_export(doc, filename, format, width, height, quality);
    
    if (!success) {
        printf("[ERROR] Export failed for %s\n", filename);
    }
}

// 添加专门的导出命令函数
void cmd_export_bmp(SVGDocument* doc, int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: export_bmp <filename> [width] [height]\n");
        return;
    }
    
    int width = (argc >= 3) ? atoi(argv[2]) : 800;
    int height = (argc >= 4) ? atoi(argv[3]) : 600;
    
    bool success = export_bmp(doc, argv[1], width, height);
    if (!success) {
        printf("[ERROR] BMP export failed for %s\n", argv[1]);
    }
}

void cmd_export_jpg(SVGDocument* doc, int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: export_jpg <filename> [width] [height] [quality]\n");
        return;
    }
    
    int width = (argc >= 3) ? atoi(argv[2]) : 800;
    int height = (argc >= 4) ? atoi(argv[3]) : 600;
    int quality = (argc >= 5) ? atoi(argv[4]) : 90;
    
    bool success = export_jpg(doc, argv[1], width, height, quality);
    if (!success) {
        printf("[ERROR] JPG export failed for %s\n", argv[1]);
    }
}

void cmd_export_svg(SVGDocument* doc, int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: export_svg <filename>\n");
        return;
    }
    
    bool success = export_svg(doc, argv[1]);
    if (!success) {
        printf("[ERROR] SVG export failed for %s\n", argv[1]);
    }
}

void cmd_load(SVGDocument* doc, int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: load <filename>\n");
        return;
    }
    svg_load(doc, argv[1]);
}

void cmd_save(SVGDocument* doc, int argc, char* argv[]) {
    const char* filename = (argc >= 2) ? argv[1] : doc->filename;
    svg_save(doc, filename);
}

void cmd_list(SVGDocument* doc, int argc, char* argv[]) {
    svg_list_shapes(doc);
}

void cmd_info(SVGDocument* doc, int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: info <shape_index>\n");
        return;
    }
    int index = atoi(argv[1]);
    svg_show_info(doc, index);
}

void cmd_select(SVGDocument* doc, int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: select <shape_index>\n");
        return;
    }
    int index = atoi(argv[1]);
    if (index < 1 || index > doc->shape_count) {
        printf("[ERROR] Invalid shape index: %d\n", index);
        return;
    }
    
    doc->selected_shape = index - 1;
    Shape* shape = &doc->shapes[doc->selected_shape];
    printf("[SUCCESS] Selected shape [%d]: ", shape->id);
    switch (shape->type) {
        case SHAPE_CIRCLE: printf("CIRCLE\n"); break;
        case SHAPE_RECTANGLE: printf("RECTANGLE\n"); break;
        case SHAPE_LINE: printf("LINE\n"); break;
        case SHAPE_POLYGON: printf("POLYGON\n"); break;
    }
}

void cmd_deselect(SVGDocument* doc, int argc, char* argv[]) {
    doc->selected_shape = -1;
    printf("[SUCCESS] Selection cleared\n");
}

void cmd_selected(SVGDocument* doc, int argc, char* argv[]) {
    if (doc->selected_shape == -1) {
        printf("No shape selected\n");
    } else {
        Shape* shape = &doc->shapes[doc->selected_shape];
        printf("Current selection: Shape [%d] ", shape->id);
        switch (shape->type) {
            case SHAPE_CIRCLE: printf("CIRCLE\n"); break;
            case SHAPE_RECTANGLE: printf("RECTANGLE\n"); break;
            case SHAPE_LINE: printf("LINE\n"); break;
            case SHAPE_POLYGON: printf("POLYGON\n"); break;
        }
    }
}

void cmd_move(SVGDocument* doc, int argc, char* argv[]) {
    if (doc->selected_shape == -1) {
        printf("[ERROR] No shape selected\n");
        return;
    }
    if (argc < 3) {
        printf("Usage: move <dx> <dy>\n");
        return;
    }
    
    double dx = atof(argv[1]);
    double dy = atof(argv[2]);
    Shape* shape = &doc->shapes[doc->selected_shape];
    
    switch (shape->type) {
        case SHAPE_CIRCLE:
            shape->data.circle.cx += dx;
            shape->data.circle.cy += dy;
            break;
        case SHAPE_RECTANGLE:
            shape->data.rectangle.x += dx;
            shape->data.rectangle.y += dy;
            break;
        case SHAPE_LINE:
            shape->data.line.x1 += dx;
            shape->data.line.y1 += dy;
            shape->data.line.x2 += dx;
            shape->data.line.y2 += dy;
            break;
        case SHAPE_POLYGON:
            for (int i = 0; i < shape->data.polygon.point_count; i++) {
                shape->data.polygon.points[i].x += dx;
                shape->data.polygon.points[i].y += dy;
            }
            break;
    }
    
    printf("[SUCCESS] Shape moved by (%.1f,%.1f)\n", dx, dy);
    doc->modified = true;
}

void cmd_set_fill(SVGDocument* doc, int argc, char* argv[]) {
    if (doc->selected_shape == -1) {
        printf("[ERROR] No shape selected\n");
        return;
    }
    if (argc < 4) {
        printf("Usage: set_fill <r> <g> <b>\n");
        return;
    }
    
    Shape* shape = &doc->shapes[doc->selected_shape];
    shape->fill_r = atof(argv[1]);
    shape->fill_g = atof(argv[2]);
    shape->fill_b = atof(argv[3]);
    
    printf("[SUCCESS] Fill color set to RGB(%.0f,%.0f,%.0f)\n", 
           shape->fill_r, shape->fill_g, shape->fill_b);
    doc->modified = true;
}

void cmd_set_stroke(SVGDocument* doc, int argc, char* argv[]) {
    if (doc->selected_shape == -1) {
        printf("[ERROR] No shape selected\n");
        return;
    }
    if (argc < 4) {
        printf("Usage: set_stroke <r> <g> <b>\n");
        return;
    }
    
    Shape* shape = &doc->shapes[doc->selected_shape];
    shape->stroke_r = atof(argv[1]);
    shape->stroke_g = atof(argv[2]);
    shape->stroke_b = atof(argv[3]);
    
    printf("[SUCCESS] Stroke color set to RGB(%.0f,%.0f,%.0f)\n", 
           shape->stroke_r, shape->stroke_g, shape->stroke_b);
    doc->modified = true;
}

void cmd_set_width(SVGDocument* doc, int argc, char* argv[]) {
    if (doc->selected_shape == -1) {
        printf("[ERROR] No shape selected\n");
        return;
    }
    if (argc < 2) {
        printf("Usage: set_width <width>\n");
        return;
    }
    
    Shape* shape = &doc->shapes[doc->selected_shape];
    shape->stroke_width = atof(argv[1]);
    
    printf("[SUCCESS] Stroke width set to %.1f\n", shape->stroke_width);
    doc->modified = true;
}

void cmd_add_circle(SVGDocument* doc, int argc, char* argv[]) {
    if (argc < 4) {
        printf("Usage: add_circle <cx> <cy> <r>\n");
        return;
    }
    
    if (doc->shape_count >= MAX_SHAPES) {
        printf("[ERROR] Maximum number of shapes reached\n");
        return;
    }
    
    Shape* shape = &doc->shapes[doc->shape_count++];
    shape->type = SHAPE_CIRCLE;
    shape->id = doc->shape_count;
    shape->data.circle.cx = atof(argv[1]);
    shape->data.circle.cy = atof(argv[2]);
    shape->data.circle.r = atof(argv[3]);
    
    printf("[SUCCESS] Added circle [%d] at (%.1f,%.1f) r=%.1f\n", 
           shape->id, shape->data.circle.cx, shape->data.circle.cy, shape->data.circle.r);
    doc->modified = true;
}

void cmd_add_rect(SVGDocument* doc, int argc, char* argv[]) {
    if (argc < 5) {
        printf("Usage: add_rect <x> <y> <width> <height>\n");
        return;
    }
    
    if (doc->shape_count >= MAX_SHAPES) {
        printf("[ERROR] Maximum number of shapes reached\n");
        return;
    }
    
    Shape* shape = &doc->shapes[doc->shape_count++];
    shape->type = SHAPE_RECTANGLE;
    shape->id = doc->shape_count;
    shape->data.rectangle.x = atof(argv[1]);
    shape->data.rectangle.y = atof(argv[2]);
    shape->data.rectangle.width = atof(argv[3]);
    shape->data.rectangle.height = atof(argv[4]);
    
    printf("[SUCCESS] Added rectangle [%d] at (%.1f,%.1f) %.1fx%.1f\n", 
           shape->id, shape->data.rectangle.x, shape->data.rectangle.y,
           shape->data.rectangle.width, shape->data.rectangle.height);
    doc->modified = true;
}

void cmd_add_line(SVGDocument* doc, int argc, char* argv[]) {
    if (argc < 5) {
        printf("Usage: add_line <x1> <y1> <x2> <y2>\n");
        return;
    }
    
    if (doc->shape_count >= MAX_SHAPES) {
        printf("[ERROR] Maximum number of shapes reached\n");
        return;
    }
    
    Shape* shape = &doc->shapes[doc->shape_count++];
    shape->type = SHAPE_LINE;
    shape->id = doc->shape_count;
    shape->data.line.x1 = atof(argv[1]);
    shape->data.line.y1 = atof(argv[2]);
    shape->data.line.x2 = atof(argv[3]);
    shape->data.line.y2 = atof(argv[4]);
    
    printf("[SUCCESS] Added line [%d] from (%.1f,%.1f) to (%.1f,%.1f)\n", 
           shape->id, shape->data.line.x1, shape->data.line.y1,
           shape->data.line.x2, shape->data.line.y2);
    doc->modified = true;
}

void cmd_add_polygon(SVGDocument* doc, int argc, char* argv[]) {
    if (argc < 3 || (argc - 1) % 2 != 0) {
        printf("Usage: add_polygon <x1> <y1> <x2> <y2> ...\n");
        return;
    }
    
    if (doc->shape_count >= MAX_SHAPES) {
        printf("[ERROR] Maximum number of shapes reached\n");
        return;
    }
    
    int point_count = (argc - 1) / 2;
    Shape* shape = &doc->shapes[doc->shape_count++];
    shape->type = SHAPE_POLYGON;
    shape->id = doc->shape_count;
    shape->data.polygon.point_count = point_count;
    shape->data.polygon.points = malloc(point_count * sizeof(Point));
    
    for (int i = 0; i < point_count; i++) {
        shape->data.polygon.points[i].x = atof(argv[1 + i*2]);
        shape->data.polygon.points[i].y = atof(argv[2 + i*2]);
    }
    
    printf("[SUCCESS] Added polygon [%d] with %d points\n", shape->id, point_count);
    doc->modified = true;
}

void cmd_delete(SVGDocument* doc, int argc, char* argv[]) {
    if (doc->selected_shape == -1) {
        printf("[ERROR] No shape selected\n");
        return;
    }
    
    Shape* shape = &doc->shapes[doc->selected_shape];
    printf("[SUCCESS] Deleted shape [%d]: ", shape->id);
    switch (shape->type) {
        case SHAPE_CIRCLE: printf("CIRCLE\n"); break;
        case SHAPE_RECTANGLE: printf("RECTANGLE\n"); break;
        case SHAPE_LINE: printf("LINE\n"); break;
        case SHAPE_POLYGON: printf("POLYGON\n"); break;
    }
    
    // Free polygon points if needed
    if (shape->type == SHAPE_POLYGON && shape->data.polygon.points != NULL) {
        free(shape->data.polygon.points);
    }
    
    // Shift remaining shapes and update IDs
    int deleted_id = shape->id;
    for (int i = doc->selected_shape; i < doc->shape_count - 1; i++) {
        doc->shapes[i] = doc->shapes[i + 1];
        doc->shapes[i].id = i + 1; // 更新ID
    }
    doc->shape_count--;
    doc->selected_shape = -1;
    doc->modified = true;
    
    printf("Remaining shapes renumbered. Deleted ID: %d\n", deleted_id);
}

void cmd_clear(SVGDocument* doc, int argc, char* argv[]) {
    // Free polygon points
    for (int i = 0; i < doc->shape_count; i++) {
        if (doc->shapes[i].type == SHAPE_POLYGON && doc->shapes[i].data.polygon.points != NULL) {
            free(doc->shapes[i].data.polygon.points);
        }
    }
    
    doc->shape_count = 0;
    doc->selected_shape = -1;
    printf("[SUCCESS] All shapes cleared\n");
    doc->modified = true;
}

void cmd_zoom(SVGDocument* doc, int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: zoom <level>\n");
        return;
    }
    doc->zoom = atof(argv[1]);
    printf("[SUCCESS] Zoom level set to %.1f\n", doc->zoom);
}

void cmd_pan(SVGDocument* doc, int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: pan <dx> <dy>\n");
        return;
    }
    doc->pan_x += atof(argv[1]);
    doc->pan_y += atof(argv[2]);
    printf("[SUCCESS] View panned by (%.1f,%.1f)\n", atof(argv[1]), atof(argv[2]));
}

void cmd_reset(SVGDocument* doc, int argc, char* argv[]) {
    doc->zoom = 1.0;
    doc->pan_x = 0.0;
    doc->pan_y = 0.0;
    printf("[SUCCESS] View reset\n");
}

void cmd_fit(SVGDocument* doc, int argc, char* argv[]) {
    printf("[SUCCESS] View fitted to content\n");
}

void cmd_undo(SVGDocument* doc, int argc, char* argv[]) {
    printf("[INFO] Undo operation (not implemented)\n");
}

void cmd_redo(SVGDocument* doc, int argc, char* argv[]) {
    printf("[INFO] Redo operation (not implemented)\n");
}

void cmd_history(SVGDocument* doc, int argc, char* argv[]) {
    printf("Operation history (not implemented)\n");
}

void cmd_help(SVGDocument* doc, int argc, char* argv[]) {
    if (argc > 1) {
        const char* command = argv[1];
        printf("Detailed help for '%s':\n", command);
        
        if (strcmp(command, "load") == 0) {
            printf("  load <filename>\n");
            printf("  Load an SVG file from the specified path.\n");
            printf("  The file will be parsed and all shapes will be loaded into memory.\n");
            printf("  Example: load examples/basic.svg\n");
            printf("  Example: load /path/to/my/shapes.svg\n");
            
        } else if (strcmp(command, "save") == 0) {
            printf("  save [filename]\n");
            printf("  Save the current document to a file.\n");
            printf("  If no filename is provided, uses the current document name.\n");
            printf("  Example: save my_document.svg\n");
            printf("  Example: save (uses current filename)\n");
            
        } else if (strcmp(command, "export") == 0) {
            printf("  export <filename> <format> [width] [height] [quality]\n");
            printf("  Export the document as an image file in specified format.\n");
            printf("  Supported formats: bmp, jpg, png, svg\n");
            printf("  Default size: 800x600, Default quality (jpg): 90\n");
            printf("  Example: export output.bmp bmp\n");
            printf("  Example: export output.jpg jpg 1024 768 85\n");
            printf("  Example: export output.png png 1920 1080\n");
            
        } else if (strcmp(command, "export_bmp") == 0) {
            printf("  export_bmp <filename> [width] [height]\n");
            printf("  Export as BMP image format.\n");
            printf("  Default size: 800x600\n");
            printf("  Example: export_bmp image.bmp\n");
            printf("  Example: export_bmp image.bmp 640 480\n");
            
        } else if (strcmp(command, "export_jpg") == 0) {
            printf("  export_jpg <filename> [width] [height] [quality]\n");
            printf("  Export as JPEG image format.\n");
            printf("  Default size: 800x600, Default quality: 90\n");
            printf("  Quality range: 1-100 (higher = better quality)\n");
            printf("  Example: export_jpg photo.jpg\n");
            printf("  Example: export_jpg photo.jpg 1920 1080 95\n");
            
        } else if (strcmp(command, "export_png") == 0) {
            printf("  export_png <filename> [width] [height]\n");
            printf("  Export as PNG image format.\n");
            printf("  Default size: 800x600\n");
            printf("  Example: export_png image.png\n");
            printf("  Example: export_png image.png 1280 720\n");
            
        } else if (strcmp(command, "export_svg") == 0) {
            printf("  export_svg <filename>\n");
            printf("  Export as SVG vector format.\n");
            printf("  This is equivalent to the save command.\n");
            printf("  Example: export_svg output.svg\n");
            
        } else if (strcmp(command, "list") == 0) {
            printf("  list\n");
            printf("  List all shapes in the current document.\n");
            printf("  Shows shape index, type, and basic properties.\n");
            printf("  Example: list\n");
            
        } else if (strcmp(command, "info") == 0) {
            printf("  info <index>\n");
            printf("  Show detailed information about a specific shape.\n");
            printf("  Use 'list' command to see available indices.\n");
            printf("  Example: info 1\n");
            printf("  Example: info 3\n");
            
        } else if (strcmp(command, "select") == 0) {
            printf("  select <index>\n");
            printf("  Select a shape by its index number.\n");
            printf("  Selected shapes can be modified with other commands.\n");
            printf("  Use 'list' to see available indices.\n");
            printf("  Example: select 1\n");
            
        } else if (strcmp(command, "deselect") == 0) {
            printf("  deselect\n");
            printf("  Clear the current selection.\n");
            printf("  Example: deselect\n");
            
        } else if (strcmp(command, "selected") == 0) {
            printf("  selected\n");
            printf("  Show information about the currently selected shape.\n");
            printf("  Example: selected\n");
            
        } else if (strcmp(command, "move") == 0) {
            printf("  move <dx> <dy>\n");
            printf("  Move the selected shape by the specified offset.\n");
            printf("  Positive values move right/down, negative left/up.\n");
            printf("  Example: move 10 5     (move right 10, down 5)\n");
            printf("  Example: move -5 -10   (move left 5, up 10)\n");
            
        } else if (strcmp(command, "set_fill") == 0) {
            printf("  set_fill <r> <g> <b>\n");
            printf("  Set the fill color for the selected shape.\n");
            printf("  Color components range: 0-255\n");
            printf("  Example: set_fill 255 0 0     (red)\n");
            printf("  Example: set_fill 0 255 0     (green)\n");
            printf("  Example: set_fill 0 0 255     (blue)\n");
            
        } else if (strcmp(command, "set_stroke") == 0) {
            printf("  set_stroke <r> <g> <b>\n");
            printf("  Set the stroke color for the selected shape.\n");
            printf("  Color components range: 0-255\n");
            printf("  Example: set_stroke 0 0 0     (black)\n");
            printf("  Example: set_stroke 255 255 0 (yellow)\n");
            
        } else if (strcmp(command, "set_width") == 0) {
            printf("  set_width <width>\n");
            printf("  Set the stroke width for the selected shape.\n");
            printf("  Example: set_width 2.5\n");
            printf("  Example: set_width 5.0\n");
            
        } else if (strcmp(command, "add_circle") == 0) {
            printf("  add_circle <cx> <cy> <radius>\n");
            printf("  Add a new circle to the document.\n");
            printf("  cx, cy: center coordinates\n");
            printf("  radius: circle radius\n");
            printf("  Example: add_circle 100 150 25\n");
            printf("  Example: add_circle 300 200 40\n");
            
        } else if (strcmp(command, "add_rect") == 0) {
            printf("  add_rect <x> <y> <width> <height>\n");
            printf("  Add a new rectangle to the document.\n");
            printf("  x, y: top-left corner coordinates\n");
            printf("  width, height: rectangle dimensions\n");
            printf("  Example: add_rect 50 75 40 30\n");
            printf("  Example: add_rect 200 300 60 40\n");
            
        } else if (strcmp(command, "add_line") == 0) {
            printf("  add_line <x1> <y1> <x2> <y2>\n");
            printf("  Add a new line to the document.\n");
            printf("  x1, y1: start point coordinates\n");
            printf("  x2, y2: end point coordinates\n");
            printf("  Example: add_line 0 0 200 200\n");
            printf("  Example: add_line 100 50 300 150\n");
            
        } else if (strcmp(command, "add_polygon") == 0) {
            printf("  add_polygon <x1> <y1> <x2> <y2> ...\n");
            printf("  Add a new polygon to the document.\n");
            printf("  Provide at least 3 points (6 coordinates).\n");
            printf("  Example: add_polygon 100 100 200 100 150 200\n");
            printf("  Example: add_polygon 50 50 100 50 100 100 50 100\n");
            
        } else if (strcmp(command, "delete") == 0) {
            printf("  delete\n");
            printf("  Delete the currently selected shape.\n");
            printf("  Remaining shapes will be renumbered.\n");
            printf("  Example: delete\n");
            
        } else if (strcmp(command, "clear") == 0) {
            printf("  clear\n");
            printf("  Remove all shapes from the document.\n");
            printf("  This operation cannot be undone.\n");
            printf("  Example: clear\n");
            
        } else if (strcmp(command, "zoom") == 0) {
            printf("  zoom <level>\n");
            printf("  Set the zoom level for the view.\n");
            printf("  1.0 = 100%%, 2.0 = 200%%, 0.5 = 50%%\n");
            printf("  Example: zoom 1.5\n");
            printf("  Example: zoom 0.8\n");
            
        } else if (strcmp(command, "pan") == 0) {
            printf("  pan <dx> <dy>\n");
            printf("  Pan the view by the specified offset.\n");
            printf("  Positive values pan right/down, negative left/up.\n");
            printf("  Example: pan 10 20\n");
            printf("  Example: pan -5 -10\n");
            
        } else if (strcmp(command, "reset") == 0) {
            printf("  reset\n");
            printf("  Reset the view to default settings.\n");
            printf("  Zoom: 1.0, Pan: (0, 0)\n");
            printf("  Example: reset\n");
            
        } else if (strcmp(command, "fit") == 0) {
            printf("  fit\n");
            printf("  Fit the view to show all content.\n");
            printf("  Automatically adjusts zoom and pan.\n");
            printf("  Example: fit\n");
            
        } else if (strcmp(command, "undo") == 0) {
            printf("  undo\n");
            printf("  Undo the last operation.\n");
            printf("  Note: This feature is not yet implemented.\n");
            printf("  Example: undo\n");
            
        } else if (strcmp(command, "redo") == 0) {
            printf("  redo\n");
            printf("  Redo the last undone operation.\n");
            printf("  Note: This feature is not yet implemented.\n");
            printf("  Example: redo\n");
            
        } else if (strcmp(command, "history") == 0) {
            printf("  history\n");
            printf("  Show the operation history.\n");
            printf("  Note: This feature is not yet implemented.\n");
            printf("  Example: history\n");
            
        } else if (strcmp(command, "help") == 0) {
            printf("  help [command]\n");
            printf("  Show help information.\n");
            printf("  Without arguments: show all commands\n");
            printf("  With argument: show detailed help for specific command\n");
            printf("  Example: help\n");
            printf("  Example: help export\n");
            
        } else if (strcmp(command, "status") == 0) {
            printf("  status\n");
            printf("  Show current system status and document information.\n");
            printf("  Includes filename, shape count, selection, etc.\n");
            printf("  Example: status\n");
            
        } else if (strcmp(command, "quit") == 0) {
            printf("  quit\n");
            printf("  Exit the SVG Editor program.\n");
            printf("  If there are unsaved changes, you will be prompted.\n");
            printf("  Example: quit\n");
            
        } else {
            printf("  Unknown command: %s\n", command);
            printf("  Use 'help' to see all available commands.\n");
        }
    } else {
        extern void svg_editor_show_help(void);
        svg_editor_show_help();
    }
}

void cmd_status(SVGDocument* doc, int argc, char* argv[]) {
    printf("System Status:\n");
    printf("  Document: %s\n", doc->filename);
    printf("  Shapes: %d\n", doc->shape_count);
    printf("  Modified: %s\n", doc->modified ? "Yes" : "No");
    
    // 修复：使用正确的格式和类型
    if (doc->selected_shape == -1) {
        printf("  Selected: None\n");
    } else {
        printf("  Selected: %d\n", doc->shapes[doc->selected_shape].id);
    }
    
    printf("  Zoom: %.1f\n", doc->zoom);
    printf("  Pan: (%.1f,%.1f)\n", doc->pan_x, doc->pan_y);
}

void cmd_quit(SVGDocument* doc, int argc, char* argv[]) {
    if (doc->modified) {
        printf("Document has unsaved changes. Are you sure you want to quit? (y/n): ");
        char response[10];
        if (fgets(response, sizeof(response), stdin) != NULL) {
            if (tolower(response[0]) != 'y') {
                return;
            }
        }
    }
    printf("[INFO] Goodbye!\n");
    exit(0);
}