#ifndef COMMANDS_H
#define COMMANDS_H

#include "gui_render.h"

// Command functions
void cmd_load(SVGDocument* doc, int argc, char* argv[]);
void cmd_save(SVGDocument* doc, int argc, char* argv[]);
void cmd_export(SVGDocument* doc, int argc, char* argv[]);
void cmd_export_bmp(SVGDocument* doc, int argc, char* argv[]);
void cmd_export_jpg(SVGDocument* doc, int argc, char* argv[]);
void cmd_export_svg(SVGDocument* doc, int argc, char* argv[]);
void cmd_export_png(SVGDocument* doc, int argc, char* argv[]);
void cmd_list(SVGDocument* doc, int argc, char* argv[]);
void cmd_info(SVGDocument* doc, int argc, char* argv[]);
void cmd_select(SVGDocument* doc, int argc, char* argv[]);
void cmd_deselect(SVGDocument* doc, int argc, char* argv[]);
void cmd_selected(SVGDocument* doc, int argc, char* argv[]);
void cmd_move(SVGDocument* doc, int argc, char* argv[]);
void cmd_set_fill(SVGDocument* doc, int argc, char* argv[]);
void cmd_set_stroke(SVGDocument* doc, int argc, char* argv[]);
void cmd_set_width(SVGDocument* doc, int argc, char* argv[]);
void cmd_add_circle(SVGDocument* doc, int argc, char* argv[]);
void cmd_add_rect(SVGDocument* doc, int argc, char* argv[]);
void cmd_add_line(SVGDocument* doc, int argc, char* argv[]);
void cmd_add_polygon(SVGDocument* doc, int argc, char* argv[]);
void cmd_delete(SVGDocument* doc, int argc, char* argv[]);
void cmd_clear(SVGDocument* doc, int argc, char* argv[]);
void cmd_zoom(SVGDocument* doc, int argc, char* argv[]);
void cmd_pan(SVGDocument* doc, int argc, char* argv[]);
void cmd_reset(SVGDocument* doc, int argc, char* argv[]);
void cmd_fit(SVGDocument* doc, int argc, char* argv[]);
void cmd_undo(SVGDocument* doc, int argc, char* argv[]);
void cmd_redo(SVGDocument* doc, int argc, char* argv[]);
void cmd_history(SVGDocument* doc, int argc, char* argv[]);
void cmd_help(SVGDocument* doc, int argc, char* argv[]);
void cmd_status(SVGDocument* doc, int argc, char* argv[]);
void cmd_quit(SVGDocument* doc, int argc, char* argv[]);

#endif