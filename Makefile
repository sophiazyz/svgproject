# 编译器和选项
CC = gcc

# 源文件和目标文件
SRCS = src/main.c src/svg_parse.c src/svg_render.c src/bmp_writer.c src/jpg_writer.c src/image.c

# 可执行文件
TARGET = svg_processor

LIBS = -lm -ljpeg

# 默认目标
all: $(TARGET)

# 链接生成可执行文件
$(TARGET): $(SRCS)
	$(CC) $(SRCS) -o $(TARGET) $(PKG_CFLAGS) $(PKG_LIBS) $(LIBS)

# 清理生成的文件
clean:
	rm -f $(TARGET)

# 安装依赖 (Ubuntu/Debian)
install-deps:
	sudo apt-get update
	sudo apt-get install libjpeg-dev

# 调试版本
debug: CFLAGS += -g -DDEBUG
debug: all

# 发布版本
release: CFLAGS += -O3 -DNDEBUG
release: all