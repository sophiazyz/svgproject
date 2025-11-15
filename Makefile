CC = gcc
CFLAGS = -g -Wall

SRC = src/main.c src/svg_parse.c src/svg_render.c src/bmp_writer.c

all: svg_editor

svg_editor: $(SRC)
	$(CC) $(SRC) -o svg_editor $(CFLAGS)


clean:
	rm -f svg_editor
