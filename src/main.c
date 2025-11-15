#include <stdio.h>
#include <string.h>

#include "../include/svg_types.h"
#include "../include/svg_parser.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <input.svg>\n", argv[1]);
        return 1;
    }

    
    SvgDocument *doc = NULL;
    if (svg_load_from_file(argv[2], &doc) != 0)
    {
        fprintf(stderr, "Failed to load SVG file: %s\n", argv[2]);
        return 1;
    }
    
    svg_print_document(doc);

    return 0;
}
