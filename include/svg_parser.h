#ifndef SVG_PARSER_H
#define SVG_PARSER_H

#include "svg_types.h"

// Parser functions
int svg_load_from_file(const char *filename, SvgDocument **doc_out);
/*
* Reads an SVG file and parses:
* - <svg ...> for canvas dimensions
* - <circle ...>, <rect ...>, <line ...>
* Returns 0 on success, sets *doc_outConsole Display:
*/

void svg_print_document(const SvgDocument *doc);
void svg_free_document(SvgDocument *doc);

// Console display functions
void svg_print_summary(const SvgDocument *doc);
void svg_print_shapes(const SvgDocument *doc);

// Helper functions for parsing
int parse_svg_dimensions(const char *line, SvgDocument *doc);
int parse_circle(const char *line, SvgDocument *doc, int *shape_id);
int parse_rect(const char *line, SvgDocument *doc, int *shape_id);
int parse_line(const char *line, SvgDocument *doc, int *shape_id);
unsigned int parse_color(const char *color_str);

#endif