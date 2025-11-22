#ifndef SVG_EDITOR_H
#define SVG_EDITOR_H

#include "gui_render.h"
#include "gui_commands.h"

void svg_editor_run(void);
void svg_editor_show_banner(void);
void svg_editor_show_help(void);
void svg_editor_process_command(const char* input);

#endif