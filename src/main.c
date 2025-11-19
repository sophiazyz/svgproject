#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/svg_types.h"
#include "../include/svg_parser.h"
#include "../include/svg_render.h"
#include "../include/jpg_writer.h"
#include "../include/image.h"


/********************* Usage *********************/
void print_usage()
{
    printf("Usage:\n");
    printf("  ./svg_editor --export_jpg input.svg output.jpg\n");
    printf("  ./svg_editor -ej input.svg output.jpg\n");
    printf("  ./svg_editor --parser input.svg\n");
    printf("  ./svg_editor -p input.svg\n");
}

/********************* Main *********************/
int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        print_usage();
        return 1;
    }

    if (strcmp(argv[1], "--export_jpg") == 0 || strcmp(argv[1], "-ej") == 0)
    {
        if (argc != 4)
        {
            print_usage();
            return 1;
        }

        char *input_file = NULL;
        char *output_file = NULL;
        int export_jpg = 0;
        int export_bmp = 0;

        // 解析命令行参数
        for (int i = 1; i < argc; i++)
        {
            if (strcmp(argv[i], "--export_jpg") == 0 || strcmp(argv[i], "-ej") == 0)
            {
                export_jpg = 1;
            }
            else if (strcmp(argv[i], "--export_bmp") == 0 || strcmp(argv[i], "-eb") == 0)
            {
                export_bmp = 1;
            }
            else if (!input_file)
            {
                input_file = argv[i];
            }
            else
            {
                output_file = argv[i];
            }
        }

        if (!input_file || !output_file)
        {
            printf("错误: 需要指定输入和输出文件\n");
            print_usage();
            return 1;
        }

        if (!export_jpg && !export_bmp)
        {
            printf("错误: 需要指定输出格式\n");
            print_usage();
            return 1;
        }

        // 解析SVG文件
        SVGShape *shapes;
        int shape_count;
        if (!parse_svg(input_file, &shapes, &shape_count))
        {
            return 1;
        }

        // 创建图像 (假设固定尺寸，实际应该从SVG中获取)
        Image *img = create_image(800, 600);

        // 渲染SVG到图像
        render_svg_to_image(img, shapes, shape_count);

        // 输出文件
        if (export_jpg)
        {
            write_jpg(output_file, img, 90);
            printf("已生成JPG文件: %s\n", output_file);
        }
        

        // 清理内存
        free(shapes);
        free_image(img);

        return 0;
    }
    else if ((strcmp(argv[1], "--parser") == 0) || strcmp(argv[1], "-p") == 0)
    {
        const char *input = argv[2];

        SvgDocument *doc = NULL;
        if (svg_load_from_file(input, &doc) != 0)
        {
            fprintf(stderr, "Failed to load SVG file: %s\n", input);
            return 1;
        }
        svg_print_document(doc);
        svg_free_document(doc);
        return 0;
    }
    else
    {
        print_usage();
        return 1;
    }
}
