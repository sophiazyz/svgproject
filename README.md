# Simple SVG Parser and Editor (C Language)

A lightweight command-line SVG editor written in C.  
It can load simple SVG files (circles, rectangles, lines), parse basic SVG transforms (translate, scale, rotate), export the rendered result to BMP and JPG files, and edit shapes interactively.

This project intentionally keeps dependencies minimal — core parsing/rendering written in plain C. JPG export requires `libjpeg` (or an equivalent JPEG library).

---

## ✨ Features

- Load simple SVG files: `<circle>`, `<rect>`, `<line>`  
- Parse basic SVG attributes including `transform` (support for `translate`, `scale`, `rotate`)  
- Maintain document canvas size (`width` / `height`)  
- Interactive CLI: list, select, move, add, delete shapes  
- Render document to bitmap and export to:
  - BMP (no external dependency)
  - JPG (requires `libjpeg`)  
- Color name → hex parsing (e.g., `red`, `blue`) and direct hex colors (`#RRGGBB`)  
- Save edited document back to SVG

---

## 📂 Supported SVG Elements & Transform Support

| SVG Tag     | Supported Attributes                                | Transform support |
|-------------|------------------------------------------------------|-------------------|
| `<circle>`  | `cx`, `cy`, `r`, `fill`                              | yes (translate/scale/rotate) |
| `<rect>`    | `x`, `y`, `width`, `height`, `fill`                  | yes (translate/scale/rotate) |
| `<line>`    | `x1`, `y1`, `x2`, `y2`, `stroke`                     | yes (translate/scale/rotate) |

**Transform details (basic support)**:

- `translate(tx, ty)` — shifts the shape by `(tx, ty)`  
- `scale(sx, sy)` — scales coordinates by `sx` (and `sy` if provided)  
- `rotate(angle [cx cy])` — rotates by `angle` degrees around origin or optional center point  

> Complex transform lists (chained transforms) are applied in order. This is a simplified implementation — not all edge cases in the SVG spec are supported.

---

## 🛠️ Build Instructions

```bash
# Build with libjpeg (JPG export)
# install libjpeg-dev first (Ubuntu/Debian example):
sudo apt install libjpeg-dev
make
```

## 📂 File Structure (This Program)

``` bash
- include/       # Header files (.h)
  - svg_types.h
  - svg_render.h
  - svg_parser.h
  - render_console.h
  - image.h
  - bmp_writer.h
  - jpg_writer.h
  - svg_editor.h
- src/           # Source code files (.c)
  - svg_render.c # Rendering logic for export functions
  - svg_parser.c # SVG file parsing implementation
  - image.c      # convert svg to bitmap
  - bmp_writer.c # BMP format export 
  - jpg_writer.c # JPG format export 
  - main_cmd.c   # Program entry point, command-line argument handling
  - svg_editor.c # interface
- assets/
  - test.svg  // some test samples
- Makefile       # Build configuration and compilation rules
- README.md      # Project description, build instructions, usage examples
- svg_processor  # Compiled objects and executable
- svg_gui        # Compiled objects and executable
```

---

## ▶️ Running the Program

### SVG parser

``` bash
(1) parse a svg image
./svg_processor --parser input.svg
./svg_processor -p input.svg

(2) convert svg format to bmp
./svg_processor --export_bmp input.svg output.bmp
./svg_processor -eb input.svg output.bmp

(3) convert svg format to jpg
./svg_processor --export_jpg input.svg output.jpg
./svg_processor -ej input.svg output.jpg
```

### SVG editor

``` bash
./svg_editor
```

Interactive prompt appears:

``` bash
=== Simple SVG Editor ===
Type 'help' for commands
>
```

### 💻 Avaliable Commands for svg_editor

``` bash
load <file.svg>                  # Load an SVG file
save <file.svg>                  # Save the current document as SVG
summary                          # Show canvas size and shape count
list                             # List all shapes
select <id>                      # Select a shape for editing
move <dx> <dy>                   # Move selected shape
delete                           # Delete selected shape
add_circle <cx> <cy> <r> [color] # Add a new circle
add_rect <x> <y> <w> <h> [color] # Add a rectangle
help                             # Show commands
quit                             # Exit program
```

## 📝 Example Session

``` bash
=== Simple SVG Editor ===
Type 'help' for commands

> help

=== SVG Editor Commands ===
load <file.svg>      - Load SVG file
save <file.svg>      - Save to SVG file
list                 - Show all shapes
summary              - Show document summary
select <id>          - Select shape for editing
move <dx> <dy>       - Move selected shape
delete               - Delete selected shape
add_circle <cx> <cy> <r> [color] - Add circle
add_rect <x> <y> <w> <h> [color] - Add rectangle
help                 - Show this help
quit                 - Exit program

> load assets/test.svg
Loaded assets/test.svg successfully

=== SVG Document Summary ===
Canvas: 800.0 x 600.0
Total shapes: 5

> add_circle 500 500 10 yellow
Circle added with ID 6
> list
=== Shapes ===
[1] RECT: position=(50.0,75.0) size=40.0x30.0 fill=#000000
[2] LINE: from (0.0,0.0) to (200.0,200.0) stroke=#000000
[3] CIRCLE: center=(300.0,200.0) radius=40.0 fill=#000000
[4] RECT: position=(200.0,300.0) size=60.0x40.0 fill=#000000
[5] LINE: from (100.0,100.0) to (200.0,200.0) stroke=#000000
[6] CIRCLE: center=(500.0,500.0) radius=10.0 fill=#FFFF00

> select 2
Selected shape [2]
> delete
Deleted shape [2]
> list
=== Shapes ===
[1] RECT: position=(50.0,75.0) size=40.0x30.0 fill=#000000
[2] CIRCLE: center=(300.0,200.0) radius=40.0 fill=#000000
[3] RECT: position=(200.0,300.0) size=60.0x40.0 fill=#000000
[4] LINE: from (100.0,100.0) to (200.0,200.0) stroke=#000000
[5] CIRCLE: center=(500.0,500.0) radius=10.0 fill=#FFFF00

> select 3
Selected shape [3]
> move 100 0
Moved shape [3] by (100.0, 0.0)
> save assets/modified_test.svg
Saved 5 shapes to assets/modified_test.svg
Saved to assets/modified_test.svg successfully
> quit
Goodbye!

```

## ⚙️ Rendering & Export Details

**Export Details:**

- BMP export: simple 24-bit BMP writer included — no external libs required.

- JPG export: uses libjpeg to compress final framebuffer to JPG.

**Rendering Details:**

- Parse SVG elements and their transforms.

- Apply transforms to shape coordinates.

- Rasterize shapes to an in-memory RGB bitmap.

- Write bitmap to BMP or compress to JPG.

- Current renderer focuses on correctness and simplicity. Antialiasing and advanced paint features are not included by default.

## ⚙️ Dependencies

- C standard library (required)
- libjpeg (optional — only for JPG export)
- Ubuntu
- sudo apt install build-essential libjpeg-dev

## 📌 Limitations & Notes

This is not a full SVG implementation — it supports a practical subset (basic shapes + transforms).

No support yet for gradients, masks, complex styling, text, or clipping.

Transform parsing is simplified — complex nested or CSS-applied transforms may not be fully supported.

Performance is fine for small-to-medium documents; very large SVGs may be slow or memory-heavy.

## 🤝 Extending the Project

### Possible future improvements

- Add support for path and stroke widths

- Implement antialiasing

- Add PNG export (libpng)

- Support more CSS-style attribute parsing

- GPU-accelerated rendering backend
