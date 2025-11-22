#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/svg_editor.h"

static SVGDocument g_document;

void svg_editor_run(void) {
    char input[MAX_COMMAND_LENGTH];
    
    svg_init(&g_document);
    svg_editor_show_banner();
    
    while (1) {
        printf("> ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        // Remove newline
        input[strcspn(input, "\n")] = 0;
        
        if (strlen(input) == 0) {
            continue;
        }
        
        svg_editor_process_command(input);
    }
    
    svg_cleanup(&g_document);
}

void svg_editor_show_banner(void) {
    printf("[INFO] Starting interactive mode\n");
    printf("========================================\n");
    printf("ENGR1010J SVG Image Processing System\n");
    printf("Version 1.0.0\n");
    printf("========================================\n");
    printf("Type 'help' for available commands\n");
    printf("Type 'quit' to exit\n");
}

void svg_editor_show_help(void) {
    printf("Available commands:\n");
    printf("  load <filename>              - Load SVG file\n");
    printf("  save [filename]              - Save current document (optional filename)\n");
    printf("  export_bmp <file> [width] [height]          - Export as BMP image\n");
    printf("  export_jpg <file> [width] [height] [quality]      - Export as JPEG image\n");
    printf("  export_svg <file>                  - Export as SVG file\n");
    printf("  list                         - List all shapes\n");
    printf("  info <index>                 - Show shape information\n");
    printf("  select <index>               - Select shape by index\n");
    printf("  deselect                     - Clear selection\n");
    printf("  selected                     - Show current selection\n");
    printf("  move <dx> <dy>               - Move selected shape\n");
    printf("  set_fill <r> <g> <b>         - Set fill color (0-255)\n");
    printf("  set_stroke <r> <g> <b>       - Set stroke color (0-255)\n");
    printf("  set_width <width>            - Set stroke width\n");
    printf("  add_circle <cx> <cy> <r>     - Add circle\n");
    printf("  add_rect <x> <y> <w> <h>     - Add rectangle\n");
    printf("  add_line <x1> <y1> <x2> <y2> - Add line\n");
    printf("  add_polygon <x1> <y1> ...    - Add polygon (at least 3 points)\n");
    printf("  delete                       - Delete selected shape\n");
    printf("  clear                        - Clear all shapes\n");
    printf("  undo                         - Undo last operation\n");
    printf("  redo                         - Redo operation\n");
    printf("  history                      - Show operation history\n");
    printf("  help [command]               - Show help information\n");
    printf("  status                       - Show system status\n");
    printf("  quit                         - Exit program\n");
}

void svg_editor_process_command(const char* input) {
    char command[MAX_COMMAND_LENGTH];
    char* argv[10];
    int argc = 0;
    
    // Tokenize input
    char* token = strtok((char*)input, " ");
    while (token != NULL && argc < 10) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }
    
    if (argc == 0) return;
    
    // Convert command to lowercase for case-insensitive comparison
    strcpy(command, argv[0]);
    for (char* p = command; *p; p++) *p = tolower(*p);
    
    // Execute command
    if (strcmp(command, "load") == 0) {
        cmd_load(&g_document, argc, argv);
    } else if (strcmp(command, "save") == 0) {
        cmd_save(&g_document, argc, argv);
    } else if (strcmp(command, "export_bmp") == 0) {
        cmd_export_bmp(&g_document, argc, argv);
    } else if (strcmp(command, "export_jpg") == 0) {
        cmd_export_jpg(&g_document, argc, argv);
    } else if (strcmp(command, "export_svg") == 0) {
        cmd_export_svg(&g_document, argc, argv);
    } else if (strcmp(command, "list") == 0) {
        cmd_list(&g_document, argc, argv);
    } else if (strcmp(command, "info") == 0) {
        cmd_info(&g_document, argc, argv);
    } else if (strcmp(command, "select") == 0) {
        cmd_select(&g_document, argc, argv);
    } else if (strcmp(command, "deselect") == 0) {
        cmd_deselect(&g_document, argc, argv);
    } else if (strcmp(command, "selected") == 0) {
        cmd_selected(&g_document, argc, argv);
    } else if (strcmp(command, "move") == 0) {
        cmd_move(&g_document, argc, argv);
    } else if (strcmp(command, "set_fill") == 0) {
        cmd_set_fill(&g_document, argc, argv);
    } else if (strcmp(command, "set_stroke") == 0) {
        cmd_set_stroke(&g_document, argc, argv);
    } else if (strcmp(command, "set_width") == 0) {
        cmd_set_width(&g_document, argc, argv);
    } else if (strcmp(command, "add_circle") == 0) {
        cmd_add_circle(&g_document, argc, argv);
    } else if (strcmp(command, "add_rect") == 0) {
        cmd_add_rect(&g_document, argc, argv);
    } else if (strcmp(command, "add_line") == 0) {
        cmd_add_line(&g_document, argc, argv);
    } else if (strcmp(command, "add_polygon") == 0) {
        cmd_add_polygon(&g_document, argc, argv);
    } else if (strcmp(command, "delete") == 0) {
        cmd_delete(&g_document, argc, argv);
    } else if (strcmp(command, "clear") == 0) {
        cmd_clear(&g_document, argc, argv);
    } else if (strcmp(command, "zoom") == 0) {
        cmd_zoom(&g_document, argc, argv);
    } else if (strcmp(command, "pan") == 0) {
        cmd_pan(&g_document, argc, argv);
    } else if (strcmp(command, "reset") == 0) {
        cmd_reset(&g_document, argc, argv);
    } else if (strcmp(command, "fit") == 0) {
        cmd_fit(&g_document, argc, argv);
    } else if (strcmp(command, "undo") == 0) {
        cmd_undo(&g_document, argc, argv);
    } else if (strcmp(command, "redo") == 0) {
        cmd_redo(&g_document, argc, argv);
    } else if (strcmp(command, "history") == 0) {
        cmd_history(&g_document, argc, argv);
    } else if (strcmp(command, "help") == 0) {
        cmd_help(&g_document, argc, argv);
    } else if (strcmp(command, "status") == 0) {
        cmd_status(&g_document, argc, argv);
    } else if (strcmp(command, "quit") == 0) {
        cmd_quit(&g_document, argc, argv);
    } else {
        printf("[ERROR] Unknown command: %s\n", argv[0]);
        printf("Type 'help' for available commands.\n");
    }
}