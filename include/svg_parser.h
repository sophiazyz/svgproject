int svg_load_from_file(const char *filename, SvgDocument **doc_out);
/*
* Reads an SVG file and parses:
* - <svg ...> for canvas dimensions
* - <circle ...>, <rect ...>, <line ...>
* Returns 0 on success, sets *doc_outConsole Display:
*/

void svg_print_document(const SvgDocument *doc);
void svg_free_document(SvgDocument *doc);