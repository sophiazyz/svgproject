#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include/svg_render_bmp.h"
#include "../include/svg_parser.h"

//-------- Utility function: Read the complete label --------//
static void read_full_tag(FILE *fp, const char *first_line, char *out_tag)
{
    strcpy(out_tag, first_line);

    while (!strchr(out_tag, '>')) {
        char buf[2048];
        if (!fgets(buf, sizeof(buf), fp)) break;
        strcat(out_tag, buf);
    }
}

//-------- Utility function: Skip whitespace --------//
static void skip_spaces(const char **p)
{
    while (isspace((unsigned char)**p))
        (*p)++;
}

//-------- Utility function: Parse attribute value, e.g., width="800" --------//
static int parse_attr_double(const char *tag, const char *name, double *out)
{
    char *pos = strstr(tag, name);
    if (!pos) return 0;

    pos = strchr(pos, '=');
    if (!pos) return 0;
    pos++;

    if (*pos != '\"') return 0; // find opening quote "
    pos++;

    *out = atof(pos);
    return 1;
}

static int parse_attr_color(const char *tag, const char *name, unsigned int *out)
{
    char *pos = strstr(tag, name);
    if (!pos) return 0;

    pos = strchr(pos, '=');
    if (!pos) return 0;
    pos++;

    if (*pos != '\"') return 0;
    pos++;

    if (*pos != '#') return 0;
    pos++;

    *out = (unsigned int)strtoul(pos, NULL, 16); // convert into hexadecimal
    return 1;
}

//-------- Create a new shape --------//
static SvgShape *new_shape(SvgShapeType type)
{
    SvgShape *s = (SvgShape *)malloc(sizeof(SvgShape));
    if (!s) return NULL;

    memset(s, 0, sizeof(SvgShape));
    s->type = type;
    s->next = NULL;
    return s;
}

//-------- Main parsing function --------//
int svg_load_from_file(const char *filename, SvgDocument **doc_out)
{
    FILE *fp = fopen(filename, "r");
    
    if (!fp) return -1;

    // test
    printf("成功打开文件: %s\n", filename);

    SvgDocument *doc = (SvgDocument *)malloc(sizeof(SvgDocument));
    if (!doc) { fclose(fp); return -1; }
    doc->width = doc->height = 0.0;
    doc->shapes = NULL;

    char line[2048];
    int shape_id = 1;

    while (fgets(line, sizeof(line), fp))
    {
        // Remove leading and trailing whitespace
        const char *p = line;
        skip_spaces(&p);

        char tag[4096];

        //---- Parse <svg> ----//
        if (strncmp(p, "<svg", 4) == 0)
        {
            read_full_tag(fp, p, tag);

            parse_attr_double(tag, "width", &doc->width);
            parse_attr_double(tag, "height", &doc->height);
        }

        //---- 解析 <circle> ----//
        else if (strncmp(p, "<circle", 7) == 0)
        {
            read_full_tag(fp, p, tag);
            SvgShape *s = new_shape(SVG_SHAPE_CIRCLE);
            if (!s) continue;

            parse_attr_double(tag, "cx", &s->data.circle.cx);
            parse_attr_double(tag, "cy", &s->data.circle.cy);
            parse_attr_double(tag, " r",  &s->data.circle.r);
            parse_attr_color(tag, "fill", &s->data.circle.fill_color);

            s->id = shape_id++;
            s->next = doc->shapes;
            doc->shapes = s;
        }

        //---- 解析 <rect> ----//
        else if (strncmp(p, "<rect", 5) == 0)
        {
            read_full_tag(fp, p, tag);

            SvgShape *s = new_shape(SVG_SHAPE_RECT);
            
            parse_attr_double(tag, " x",      &s->data.rect.x);
            parse_attr_double(tag, " y",      &s->data.rect.y);
            parse_attr_double(tag, "width",  &s->data.rect.width);
            parse_attr_double(tag, "height", &s->data.rect.height);
            parse_attr_color(tag, "fill",    &s->data.rect.fill_color);
            s->id = shape_id++;
            s->next = doc->shapes;
            doc->shapes = s;
        }

        //---- 解析 <line> ----//
        else if (strncmp(p, "<line", 5) == 0)
        {
            read_full_tag(fp, p, tag);

            SvgShape *s = new_shape(SVG_SHAPE_LINE);

            parse_attr_double(tag, "x1", &s->data.line.x1);
            parse_attr_double(tag, "y1", &s->data.line.y1);
            parse_attr_double(tag, "x2", &s->data.line.x2);
            parse_attr_double(tag, "y2", &s->data.line.y2);
            parse_attr_color(tag, "stroke", &s->data.line.stroke_color);
            s->id = shape_id++;
            s->next = doc->shapes;
            doc->shapes = s;
        }
    }

    fclose(fp);
    *doc_out = doc;
    return 0;
}

//-------- 打印svg内容 --------//
void svg_print_document(const SvgDocument *doc)
{
    if (!doc) {
        printf("SVG Document: (null)\n");
        return;
    }

    printf("SVG Document: width=%.2f, height=%.2f\n",
           doc->width, doc->height);

    // 统计数量
    int count = 0;
    const SvgShape *s = doc->shapes;
    while (s) {
        count++;
        s = s->next;
    }

    printf("Total shapes: %d\n", count);

    s = doc->shapes;
    while (s) {
        switch (s->type) {
        case SVG_SHAPE_CIRCLE:
            printf("[%d] CIRCLE: cx=%.2f, cy=%.2f, r=%.2f, fill=#%06X\n",
                   s->id,
                   s->data.circle.cx,
                   s->data.circle.cy,
                   s->data.circle.r,
                   s->data.circle.fill_color);
            break;

        case SVG_SHAPE_RECT:
            printf("[%d] RECT: x=%.2f, y=%.2f, width=%.2f, height=%.2f, fill=#%06X\n",
                   s->id,
                   s->data.rect.x,
                   s->data.rect.y,
                   s->data.rect.width,
                   s->data.rect.height,
                   s->data.rect.fill_color);
            break;

        case SVG_SHAPE_LINE:
            printf("[%d] LINE: from (%.2f,%.2f) to (%.2f,%.2f), stroke=#%06X\n",
                   s->id,
                   s->data.line.x1,
                   s->data.line.y1,
                   s->data.line.x2,
                   s->data.line.y2,
                   s->data.line.stroke_color);
            break;
        }

        s = s->next;
    }
}

//-------- 释放文档 --------//
void svg_free_document(SvgDocument *doc)
{
    if (!doc) return;

    SvgShape *cur = doc->shapes;
    while (cur)
    {
        SvgShape *next = cur->next;
        free(cur);
        cur = next;
    }

    free(doc);
}
