# FontKit v0.1

**FontKit** is a lightweight, dependency-free font rendering library written in C. It provides TrueType font loading and high-quality glyph rasterization without requiring external libraries like FreeType.

## Features

- ✅ **No Dependencies**: Pure C implementation, no external font libraries required
- ✅ **TrueType Support**: Native TrueType font parsing and rendering
- ✅ **High Quality**: Supersampled antialiasing with configurable quality levels
- ✅ **Font Styles**: Bold, italic, underline, and strikethrough support
- ✅ **Color Support**: Full RGBA color handling with predefined color constants
- ✅ **Cross-Platform**: Windows (Win32) and Linux (X11) platform backends
- ✅ **TUI Ready**: Perfect for terminal UI applications
- ✅ **Clean API**: Single header file, static library distribution
- ✅ **Customizable**: Gamma correction, hinting modes, DPI control

## Quick Start

### Building

```bash
# Build library
make

# Build examples
make demo
make tui
make examples

# Create distribution package
make dist

# Install system-wide (optional)
sudo make install
```

### Basic Usage

```c
#include <FontKit.h>

int main() {
    // Initialize
    fk_init();
    
    // Load font
    FK_Font *font = fk_load_font(FK_FONT_FAMILY_PRIMARY, 24);
    
    // Render glyph
    FK_RenderOptions opts = {
        .quality = FK_QUALITY_HIGH,
        .hinting = FK_HINT_NORMAL,
        .gamma = 1.8f,
        .style = FK_STYLE_BOLD,
        .color = FK_COLOR_BLACK
    };
    
    FK_Glyph *glyph = fk_render_glyph(font, 'A', &opts);
    
    // Export to image
    fk_export_glyph_ppm(glyph, "output.ppm");
    
    // Cleanup
    fk_free_glyph(glyph);
    fk_free_font(font);
    fk_shutdown();
    
    return 0;
}
```

### Compile Your Program

```bash
# Link with FontKit
gcc myprogram.c -o myprogram -L./dist/lib -lFontKit -lm

# With platform support (Windows)
gcc myprogram.c -o myprogram.exe -L./dist/lib -lFontKit -lgdi32 -lm

# With platform support (Linux)
gcc myprogram.c -o myprogram -L./dist/lib -lFontKit -lX11 -lm
```

## Font Configuration

The primary font path is configured in `FontKit.h`:

```c
#define FK_FONT_FAMILY_PRIMARY   "assets/fonts/truetype/Roboto-Regular.ttf"
#define FK_FONT_FAMILY_FALLBACK  "Segoe UI, Arial, sans-serif"
```

You can override this at runtime:

```c
FK_Font *font = fk_load_font("path/to/your/font.ttf", 24);
```

## Font Styles

FontKit supports multiple font styles that can be combined:

```c
// Normal
fk_set_font_style(font, FK_STYLE_NORMAL);

// Bold
fk_set_font_style(font, FK_STYLE_BOLD);

// Italic
fk_set_font_style(font, FK_STYLE_ITALIC);

// Bold + Italic
fk_set_font_style(font, FK_STYLE_BOLD | FK_STYLE_ITALIC);

// Underline
fk_set_font_style(font, FK_STYLE_UNDERLINE);

// Strikethrough
fk_set_font_style(font, FK_STYLE_STRIKETHROUGH);
```

## Color Support

FontKit provides rich color handling:

```c
// Predefined colors
FK_Color red = FK_COLOR_RED;
FK_Color green = FK_COLOR_GREEN;
FK_Color blue = FK_COLOR_BLUE;

// Create from RGB
FK_Color custom = fk_color_rgb(128, 64, 255);

// Create from RGBA (with transparency)
FK_Color transparent = fk_color_rgba(255, 0, 0, 128);

// Parse from hex string
FK_Color hex1 = fk_color_from_hex("#FF0000");
FK_Color hex2 = fk_color_from_hex("00FF00");

// Convert to 32-bit integer
uint32_t color_value = fk_color_to_u32(red);
```

## Rendering Quality

Control antialiasing quality:

```c
FK_RenderOptions opts = {
    .quality = FK_QUALITY_ULTRA,  // 8x supersampling
    .hinting = FK_HINT_FULL,      // Full grid fitting
    .gamma = 1.8f,                // Gamma correction
    .subpixel = 1                 // Subpixel rendering
};
```

Quality levels:
- `FK_QUALITY_DRAFT` - 1x sampling (fastest)
- `FK_QUALITY_NORMAL` - 2x sampling (default)
- `FK_QUALITY_HIGH` - 4x sampling (good quality)
- `FK_QUALITY_ULTRA` - 8x sampling (best quality)

## Platform Support

### Windows (Win32)

```c
#include <FontKit.h>
#include "platform/platform.h"

int main() {
    fk_init();
    fk_platform_init();
    
    FK_Surface *surface = fk_surface_create(800, 600, "My App");
    FK_Font *font = fk_load_font("font.ttf", 24);
    
    while (!fk_surface_should_close(surface)) {
        fk_surface_clear(surface, 0xFFFFFF);
        fk_surface_draw_text(surface, font, "Hello!", 10, 10, 0x000000);
        fk_surface_present(surface);
        fk_surface_poll_events();
    }
    
    fk_surface_destroy(surface);
    fk_free_font(font);
    fk_platform_shutdown();
    fk_shutdown();
    
    return 0;
}
```

### Linux (X11)

Same code as Windows, but compile with:

```bash
gcc app.c -o app -I../include -L../dist/lib -lFontKit -lX11 -lm
```

### Terminal UI (TUI)

FontKit is perfect for TUI applications:

```c
// Create TUI buffer
TUIBuffer *tui = tui_create(80, 24);

// Draw colored text
tui_draw_text(tui, 10, 5, "Status: OK", FK_COLOR_GREEN);
tui_draw_box(tui, 5, 3, 70, 18, FK_COLOR_CYAN);

// Render to terminal
tui_render(tui);
```

See `examples/tui_demo.c` for a complete example.

## Examples

### Basic Demo
```bash
make demo
./build/demo assets/fonts/truetype/Roboto-Regular.ttf 48
```

Generates:
- `output_normal.ppm` - Normal text
- `output_bold.ppm` - Bold text
- `output_italic.ppm` - Italic text
- `output_bold_italic.ppm` - Bold + Italic

### TUI Demo
```bash
make tui
./build/tui_demo
```

Shows a terminal-based UI with:
- Colored text
- Box drawing
- Menu rendering
- Status display



# FontKit Roadmap

## Feature Roadmap from v0.1 (MVP) to v1.0 (Production-Ready)

This roadmap outlines the planned features and milestones for FontKit, a typographic system designed to evolve efficiently without overengineering in the early stages. Each milestone builds on the previous one, ensuring a useful and demonstrable result at every step.

---

## 🧱 v0.1 — Core Foundation (Minimal Working Engine)

**Goal:** Load and rasterize fonts without external dependencies.

### Key Features:
- Public API (`FontKit.h`) finalized.
- Core types: `fk_font_t`, `fk_glyph_t`, `fk_bitmap_t`.
- Basic rasterizer for monochrome bitmaps.
- Bitmap font loader.
- UTF-8 decoder.
- PPM output backend for quick visual tests.
- Demo program rendering single glyphs.

### Deliverables:
- `demo.c` renders “A” from a bitmap font.
- Static library `libFontKit.a` builds cleanly.

**Milestone Result:** A minimal but self-contained font engine that can load a font and render text into a grayscale buffer.

---

## 🧩 v0.2 — Modular Loader System + Basic Vector Support

**Goal:** Support multiple formats, unifying all through a common API.

### Key Features:
- Unified loader registration (plugins).
- Mini vector format loader (`minivec_loader.c`).
- Simple scanline rasterizer for curves.
- Add `fk_rasterize_glyph()` scale + oversample.
- Core error handling system (`fk_set_error`, `fk_get_last_error`).
- UTF-8 iteration helper for multi-character strings.

### Deliverables:
- Demo renders “HELLO” using both bitmap and vector fonts.
- Error-safe API.

**Milestone Result:** FontKit can load different font formats through modular loaders.

---

## 🎨 v0.3 — TrueType (TTF) Parsing Prototype

**Goal:** Read actual TTF glyphs and convert them to vector paths.

### Key Features:
- Parse `head`, `maxp`, `loca`, `glyf`, and `cmap` tables.
- Extract simple contour glyphs (no hinting yet).
- Map Unicode → glyph index.
- Convert TTF curves to FontKit’s `fk_path_t`.

### Deliverables:
- Demo renders glyphs from `Roboto-Regular.ttf`.
- Metrics printed in console.

**Milestone Result:** FontKit moves from a “toy engine” to real-world font support.

---

## 🧠 v0.4 — Text Layout and Metrics

**Goal:** Support line layout, kerning, and alignment.

### Key Features:
- Font metrics structure (`ascender`, `descender`, `unitsPerEm`).
- Horizontal advance width per glyph.
- Simple kerning support (parse TTF `kern` table).
- Text layout engine with:
  - `fk_layout_text(const char *utf8, fk_text_layout_t *out)`
  - Alignment (left, center, right).
  - Line wrapping by width.
- Text measurement API.

### Deliverables:
- `advanced.c` renders multi-line text with kerning.
- API for measuring and wrapping text.

**Milestone Result:** Text can now be arranged intelligently — suitable for building UI or terminal text renderers.

---

## ⚙️ v0.5 — Improved Rasterization and Visual Fidelity

**Goal:** Clean, sharp output for multiple font sizes.

### Key Features:
- Antialiasing (supersampling).
- Gamma correction.
- Optional subpixel rendering (RGB).
- Signed Distance Field (SDF) generator (optional).
- Vector stroking and outline rendering.

### Deliverables:
- Demo shows smooth scaling from 8–64px.
- Output quality comparable to FreeType grayscale.

**Milestone Result:** FontKit now delivers visually smooth text with adjustable quality trade-offs.

---

## 🧰 v0.6 — System Integration and Font Discovery

**Goal:** Integrate with OS and other projects.

### Key Features:
- Scan `/usr/share/fonts`, `C:\Windows\Fonts`, etc.
- API:
  - `fk_font_t* fk_load_system_font(const char *family, const char *style);`
- Font fallback chain (register multiple fonts).
- Glyph cache for performance.
- Memory arena allocator for zero-fragmentation.

### Deliverables:
- Demo can render a string in Roboto, falling back to an Emoji font.
- Faster repeated rendering due to caching.

**Milestone Result:** FontKit begins to behave like a production engine.

---

## 🧩 v0.7 — OpenGL / Vulkan Rendering Backend

**Goal:** Hardware-accelerated text.

### Key Features:
- Texture atlas for glyphs.
- GPU renderer for text quads.
- Shader-based SDF rendering (for crisp scaling).
- Optional 

2D batch renderer for UIs.

### Deliverables:
- `gl_demo.c` displays text on-screen using OpenGL.
- `fk_render_text_gl()` draws text directly.

**Milestone Result:** FontKit can be embedded in GUIs, games, or your GraphiKit / AlphaGUI systems.

---

## 💡 v0.8 — Advanced Typography and OpenType Features

**Goal:** Full typographic control.

### Key Features:
- Parse GSUB and GPOS tables (ligatures, contextual alternates).
- Implement shaping for Latin and simple RTL scripts.
- Add composite glyph support (accents, ligatures).
- Configurable rendering features (ligatures on/off, stylistic sets).

### Deliverables:
- Demo renders “office” showing ligature ﬁ.
- Visual test suite for OpenType features.

**Milestone Result:** Typography quality matches major font engines for standard Latin scripts.

---

## 🔍 v0.9 — Tooling and CLI Utilities

**Goal:** Add developer and test tools.

### Key Features:
- Command-line inspector (`fontkitcli`):
  - `./fontkitcli --font Roboto.ttf --char A --size 48 --out A.ppm`
- Unit tests for UTF-8 decoding, raster correctness, and metrics.
- Visual regression test images.
- Doxygen-generated API documentation.

### Deliverables:
- Automated test builds.
- Basic CI-ready repository.

**Milestone Result:** FontKit becomes maintainable and testable for long-term growth.

---

## 🏁 v1.0 — Production Release

**Goal:** Stable, performant, documented, cross-platform font library.

### Key Features:
- Full TTF/OTF + WOFF support.
- Optimized rasterizer with caching.
- Multi-threaded glyph rasterization.
- Comprehensive documentation and examples.
- Clean API freeze.

### Deliverables:
- `FontKit.h` becomes the official public API.
- Cross-platform builds (Windows, Linux, macOS).
- Integration demo with GraphiKit or AlphaGUI showing live rendered text.

**Milestone Result:** FontKit is a complete, open, dependency-free font engine suitable for GUI frameworks, embedded OSes, and custom renderers.

---

## ✨ Optional “Beyond 1.0” Goals

- WebAssembly build for browser text rendering.
- FontKit scripting API (Lua / C API extensions).
- Font editor / visual debugger.
- SVG-in-OTF support.
- GPU compute-based SDF generator.

---
