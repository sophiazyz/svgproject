# 编译器和选项
CC = gcc

LIBS = -lm -ljpeg -lSDL2 -lSDL2_ttf

# 默认构建两个版本
all: svg_processor svg_gui

# 命令行版本 - 生成 ./svg_processor
svg_processor: src/main_cmd.c src/svg_parser.c src/svg_render.c src/bmp_writer.c src/jpg_writer.c src/image.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	@echo "命令行版本构建完成: ./svg_processor"

# GUI版本 - 生成 ./svg_gui
svg_gui: src/main_gui.c src/gui_render.c src/svg_editor.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	@echo "GUI版本构建完成: ./svg_gui"

# 只构建命令行版本
cli: svg_processor

# 只构建GUI版本
gui: svg_gui


# 清理生成的文件
clean:
	rm -f svg_processor svg_gui

# 安装依赖 (Ubuntu/Debian)
install-deps-ubuntu:
	sudo apt-get update
	sudo apt-get install libsdl2-dev libsdl2-ttf-dev

# 安装依赖 (macOS)
install-deps-macos:
	brew install sdl2 sdl2_ttf


# 显示帮助信息
help:
	@echo "可用命令:"
	@echo "  make all          - 构建两个版本 (默认)"
	@echo "  make svg_processor - 只构建命令行版本"
	@echo "  make svg_gui       - 只构建GUI版本"
	@echo "  make cli          - 只构建命令行版本"
	@echo "  make gui          - 只构建GUI版本"
	@echo "  make clean        - 清理构建文件"
	@echo "  make install-deps-ubuntu - 安装依赖 (Ubuntu)"
	@echo "  make install-deps-macos  - 安装依赖 (macOS)"
	@echo ""
	@echo "使用方法:"
	@echo "  ./svg_processor    - 运行命令行版本"
	@echo "  ./svg_gui          - 运行GUI版本"

.PHONY: all cli gui clean install-deps-ubuntu install-deps-macos help