# FontKit Project Structure

```
FontKit/
│
├── include/                      # Public API (exposed to users)
│   └── FontKit.h                 # Single public header
│
├── src/                          # Private implementation (hidden)
│   │
│   ├── core/                     # Core engine
│   │   ├── fontkit_core.c        # Main implementation
│   │   ├── fontkit_core.h        # Internal core header
│   │   ├── fontkit_types.h       # Internal type definitions
│   │   └── fontkit_error.c       # Error handling
│   │
│   ├── loaders/                  # Font format loaders
│   │   ├── ttf_loader.c          # TrueType parser
│   │   ├── ttf_loader.h
│   │   ├── ttf_tables.c          # TrueType table parsing
│   │   ├── bitmap_loader.c       # Bitmap font support
│   │   └── bitmap_loader.h
│   │
│   ├── rasterizer/               # Glyph rasterization
│   │   ├── rasterizer.c          # Main rasterizer
│   │   ├── rasterizer.h
│   │   ├── bezier.c              # Curve processing
│   │   └── bezier.h
│   │
│   └── utils/                    # Utilities
│       ├── memory.c              # Memory management
│       ├── memory.h
│       ├── utf8.c                # UTF-8 handling
│       └── utf8.h
│
├── build/                        # Build artifacts (gitignore)
│   └── obj/                      # Object files
│
├── dist/                         # Distribution package
│   ├── include/
│   │   └── FontKit.h
│   └── lib/
│       └── libFontKit.a
│
├── examples/                     # Usage examples
│   ├── demo.c                    # Basic demo
│   └── advanced.c                # Advanced features demo
│
├── assets/                       # Test assets
│   └── fonts/
│       └── truetype/
│           └── Roboto-Regular.ttf
│
├── Makefile                      # Build system
├── README.md                     # Documentation
└── LICENSE                       # License file
```

## Build Commands

```bash
# Build static library
make

# Build and run demo
make demo

# Create distribution package
make dist

# Clean build artifacts
make clean

# Install to system (optional)
sudo make install
```

## Usage Example

```c
#include <FontKit.h>

int main() {
    // Initialize library
    fk_init();
    
    // Load font
    FK_Font *font = fk_load_font(FK_FONT_FAMILY_PRIMARY, 24);
    
    // Render a glyph
    FK_RenderOptions opts = {
        .quality = FK_QUALITY_HIGH,
        .hinting = FK_HINT_NORMAL,
        .gamma = 1.8f,
        .subpixel = 0
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

## Distribution

After building, `dist/` contains:
- `include/FontKit.h` - Public API header
- `lib/libFontKit.a` - Static library

Users only need these files to use FontKit in their projects.