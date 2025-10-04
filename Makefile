# FontKit Makefile - Complete build system

# CFLAGS  := -Wall -Wextra -std=c11 -Iinclude -Isrc -Isrc/core -Isrc/loaders -Isrc/rasterizer -Isrc/utils


# Compiler settings
CC = gcc
AR = ar
CFLAGS  := -Wall -Wextra -std=c11 -Iinclude -Isrc -Isrc/core -Isrc/loaders -Isrc/rasterizer -Isrc/utils
LDFLAGS = -lm

# Directories
SRC_DIR = src
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
DIST_DIR = dist
EXAMPLE_DIR = examples

# Library output
LIB_NAME = libFontKit.a
LIB_PATH = $(DIST_DIR)/lib/$(LIB_NAME)

# Source files
CORE_SRC = $(wildcard $(SRC_DIR)/core/*.c)
LOADER_SRC = $(wildcard $(SRC_DIR)/loaders/*.c)
RAST_SRC = $(wildcard $(SRC_DIR)/rasterizer/*.c)
UTIL_SRC = $(wildcard $(SRC_DIR)/utils/*.c)

ALL_SRC = $(CORE_SRC) $(LOADER_SRC) $(RAST_SRC) $(UTIL_SRC)

# Object files
CORE_OBJ = $(patsubst $(SRC_DIR)/core/%.c,$(OBJ_DIR)/core/%.o,$(CORE_SRC))
LOADER_OBJ = $(patsubst $(SRC_DIR)/loaders/%.c,$(OBJ_DIR)/loaders/%.o,$(LOADER_SRC))
RAST_OBJ = $(patsubst $(SRC_DIR)/rasterizer/%.c,$(OBJ_DIR)/rasterizer/%.o,$(RAST_SRC))
UTIL_OBJ = $(patsubst $(SRC_DIR)/utils/%.c,$(OBJ_DIR)/utils/%.o,$(UTIL_SRC))

ALL_OBJ = $(CORE_OBJ) $(LOADER_OBJ) $(RAST_OBJ) $(UTIL_OBJ)

# Examples
DEMO_SRC = $(EXAMPLE_DIR)/demo.c
DEMO_BIN = $(BUILD_DIR)/demo

# Phony targets
.PHONY: all clean dist install demo help

# Default target
all: $(LIB_PATH)
	@echo "✓ Build complete: $(LIB_PATH)"

# Create directories
$(OBJ_DIR)/core $(OBJ_DIR)/loaders $(OBJ_DIR)/rasterizer $(OBJ_DIR)/utils:
	@mkdir -p $@

$(DIST_DIR)/lib $(DIST_DIR)/include:
	@mkdir -p $@

# Compile core sources
$(OBJ_DIR)/core/%.o: $(SRC_DIR)/core/%.c | $(OBJ_DIR)/core
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Compile loader sources
$(OBJ_DIR)/loaders/%.o: $(SRC_DIR)/loaders/%.c | $(OBJ_DIR)/loaders
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Compile rasterizer sources
$(OBJ_DIR)/rasterizer/%.o: $(SRC_DIR)/rasterizer/%.c | $(OBJ_DIR)/rasterizer
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Compile utility sources
$(OBJ_DIR)/utils/%.o: $(SRC_DIR)/utils/%.c | $(OBJ_DIR)/utils
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Create static library
$(LIB_PATH): $(ALL_OBJ) | $(DIST_DIR)/lib $(DIST_DIR)/include
	@echo "AR $(LIB_NAME)"
	@$(AR) rcs $@ $(ALL_OBJ)
	@cp include/FontKit.h $(DIST_DIR)/include/
	@echo "✓ Library created: $@"
	@echo "✓ Header copied: $(DIST_DIR)/include/FontKit.h"

# Build demo
demo: $(LIB_PATH)
	@echo "Building demo..."
	@$(CC) $(CFLAGS) $(DEMO_SRC) -o $(DEMO_BIN) -L$(DIST_DIR)/lib -lFontKit $(LDFLAGS)
	@echo "✓ Demo built: $(DEMO_BIN)"
	@echo "Run with: $(DEMO_BIN)"

# Create distribution package
dist: $(LIB_PATH)
	@echo "Creating distribution package..."
	@mkdir -p $(DIST_DIR)/lib $(DIST_DIR)/include $(DIST_DIR)/examples
	@cp include/FontKit.h $(DIST_DIR)/include/
	@cp $(EXAMPLE_DIR)/*.c $(DIST_DIR)/examples/ 2>/dev/null || true
	@echo "# FontKit v0.1" > $(DIST_DIR)/README.md
	@echo "" >> $(DIST_DIR)/README.md
	@echo "## Usage" >> $(DIST_DIR)/README.md
	@echo '```c' >> $(DIST_DIR)/README.md
	@echo '#include <FontKit.h>' >> $(DIST_DIR)/README.md
	@echo "" >> $(DIST_DIR)/README.md
	@echo "// Link with: -L. -lFontKit -lm" >> $(DIST_DIR)/README.md
	@echo '```' >> $(DIST_DIR)/README.md
	@echo "✓ Distribution ready in $(DIST_DIR)/"

# Install to system (optional)
install: $(LIB_PATH)
	@echo "Installing FontKit..."
	@install -d /usr/local/lib
	@install -d /usr/local/include
	@install -m 644 $(LIB_PATH) /usr/local/lib/
	@install -m 644 include/FontKit.h /usr/local/include/
	@echo "✓ Installed to /usr/local/"

# Clean build artifacts
clean:
	@echo "Cleaning..."
	@rm -rf $(BUILD_DIR) $(DIST_DIR)
	@echo "✓ Clean complete"

# Help
help:
	@