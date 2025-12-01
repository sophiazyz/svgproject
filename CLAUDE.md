# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a **Simple SVG Parser and Editor** written in C, designed as a command-line tool for processing and editing SVG files. The project supports both a CLI version (`svg_processor`) and a GUI version (`svg_gui`), with the ability to parse basic SVG elements, apply transformations, and export to BMP/JPG formats.

## Build & Development Commands

### Prerequisites
```bash
# Ubuntu/Debian
sudo apt install build-essential libjpeg-dev libsdl2-dev libsdl2-ttf-dev

# macOS (using Homebrew)
brew install jpeg sdl2 sdl2_ttf
```

### Build Targets
```bash
make all           # Build both CLI and GUI versions
make svg_processor # Build CLI version only
make svg_gui       # Build GUI version only
make clean         # Clean all build artifacts
```

### Usage
```bash
# CLI version
./svg_processor --parser input.svg
./svg_processor --export_bmp input.svg output.bmp
./svg_processor --export_jpg input.svg output.jpg

# GUI version (interactive editor)
./svg_gui
```

## Architecture

### Core Components
- **SVG Parser** (`src/svg_parser.c`, `include/svg_parser.h`): Handles SVG file parsing and loading
- **Rendering Engine** (`src/svg_render.c`, `include/svg_render.h`): Core rendering with transformation matrices
- **Image Export**: BMP (`src/bmp_writer.c`) and JPG (`src/jpg_writer.c`) exporters
- **Interactive Editor** (`src/svg_editor.c`): GUI-based shape editing interface

### Data Structure Design
- **Linked Lists**: Used for shape management throughout the document
- **Union-based Shape Storage**: Type polymorphism for different SVG shapes (circle, rect, line)
- **Transform Matrix System**: 3x3 transformation matrices for coordinate transformations
- **RGB Color Structure**: Supports hex color parsing and manipulation

### File Organization
```
/include/           # Header files with type definitions and interfaces
/src/              # Implementation files
/assets/           # Test SVG files and export examples
.vscode/           # Debugging configuration for Windows/MinGW
```

### Supported SVG Elements
- **Circle**: `cx`, `cy`, `r`, `fill` attributes
- **Rectangle**: `x`, `y`, `width`, `height`, `fill` attributes
- **Line**: `x1`, `y1`, `x2`, `y2`, `stroke` attributes

### Transform Support
- `translate(tx, ty)` - Shape translation
- `scale(sx, sy)` - Coordinate scaling
- `rotate(angle [cx cy])` - Rotation around origin or center
- Complex transform chains (applied in order)

### Interactive Editor Commands
- `load <file>` - Load SVG file
- `save <file>` - Save to SVG format
- `list` - Show all shapes with IDs
- `select <id>` - Select shape for editing
- `move <dx> <dy>` - Move selected shape
- `delete` - Delete selected shape
- `add_circle <cx> <cy> <r> [color]` - Add new circle
- `add_rect <x> <y> <w> <h> [color]` - Add new rectangle

## Key Dependencies

- **libjpeg**: Required for JPG export functionality
- **SDL2**: Required only for GUI version
- **SDL2_ttf**: Required only for GUI text rendering
- **Standard C library**: All other functionality uses built-in libraries

## Development Notes

- The codebase follows a modular C architecture with clear separation of concerns
- Mixed language comments (Chinese/English) for educational context
- BMP writer uses no external dependencies for maximum compatibility
- Performance may degrade with very large SVG documents
- Limited to basic SVG shape types (no support for gradients, masks, or complex styling)