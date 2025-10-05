/* ============================================================================
 * examples/tui_demo.c - Terminal UI Demo for FontKit
 * Demonstrates using FontKit for TUI applications
 * ========================================================================== */
#include <FontKit.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 24

/* Simple TUI buffer */
typedef struct {
    char *buffer;
    FK_Color *colors;
    int width;
    int height;
} TUIBuffer;

TUIBuffer* tui_create(int width, int height) {
    TUIBuffer *tui = calloc(1, sizeof(TUIBuffer));
    tui->width = width;
    tui->height = height;
    tui->buffer = calloc(width * height, sizeof(char));
    tui->colors = calloc(width * height, sizeof(FK_Color));
    
    /* Fill with default color */
    for (int i = 0; i < width * height; i++) {
        tui->buffer[i] = ' ';
        tui->colors[i] = FK_COLOR_WHITE;
    }
    
    return tui;
}

void tui_destroy(TUIBuffer *tui) {
    if (!tui) return;
    free(tui->buffer);
    free(tui->colors);
    free(tui);
}

void tui_draw_text(TUIBuffer *tui, int x, int y, const char *text, FK_Color color) {
    if (!tui || !text) return;
    
    int len = strlen(text);
    for (int i = 0; i < len && x + i < tui->width; i++) {
        if (y >= 0 && y < tui->height) {
            tui->buffer[y * tui->width + x + i] = text[i];
            tui->colors[y * tui->width + x + i] = color;
        }
    }
}

void tui_draw_box(TUIBuffer *tui, int x, int y, int w, int h, FK_Color color) {
    /* Draw box borders using ASCII characters */
    for (int i = 0; i < w; i++) {
        if (y >= 0 && y < tui->height && x + i < tui->width) {
            tui->buffer[y * tui->width + x + i] = '-';
            tui->colors[y * tui->width + x + i] = color;
        }
        if (y + h - 1 >= 0 && y + h - 1 < tui->height && x + i < tui->width) {
            tui->buffer[(y + h - 1) * tui->width + x + i] = '-';
            tui->colors[(y + h - 1) * tui->width + x + i] = color;
        }
    }
    
    for (int i = 0; i < h; i++) {
        if (y + i >= 0 && y + i < tui->height) {
            if (x >= 0 && x < tui->width) {
                tui->buffer[(y + i) * tui->width + x] = '|';
                tui->colors[(y + i) * tui->width + x] = color;
            }
            if (x + w - 1 < tui->width) {
                tui->buffer[(y + i) * tui->width + x + w - 1] = '|';
                tui->colors[(y + i) * tui->width + x + w - 1] = color;
            }
        }
    }
}

void tui_render(TUIBuffer *tui) {
    /* Clear screen */
    printf("\033[2J\033[H");
    
    /* Render buffer */
    for (int y = 0; y < tui->height; y++) {
        for (int x = 0; x < tui->width; x++) {
            FK_Color c = tui->colors[y * tui->width + x];
            
            /* ANSI color codes */
            printf("\033[38;2;%d;%d;%dm", c.r, c.g, c.b);
            printf("%c", tui->buffer[y * tui->width + x]);
        }
        printf("\033[0m\n");
    }
}

int main(void) {
    printf("FontKit TUI Demo\n");
    printf("================\n\n");
    
    /* Initialize FontKit */
    if (fk_init() != FK_OK) {
        fprintf(stderr, "Failed to initialize FontKit\n");
        return 1;
    }
    
    /* Create TUI buffer */
    TUIBuffer *tui = tui_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    
    /* Draw UI elements */
    tui_draw_box(tui, 2, 2, 76, 20, FK_COLOR_CYAN);
    tui_draw_text(tui, 4, 3, "FontKit TUI Application", FK_COLOR_GREEN);
    tui_draw_text(tui, 4, 4, "======================", FK_COLOR_GREEN);
    
    tui_draw_text(tui, 4, 6, "Menu:", FK_COLOR_YELLOW);
    tui_draw_text(tui, 6, 7, "1. Normal text", FK_COLOR_WHITE);
    tui_draw_text(tui, 6, 8, "2. Bold text (simulated)", FK_COLOR_WHITE);
    tui_draw_text(tui, 6, 9, "3. Colored text", FK_COLOR_RED);
    
    tui_draw_text(tui, 4, 11, "Status: Ready", FK_COLOR_GREEN);
    tui_draw_text(tui, 4, 13, "FontKit Version: ", FK_COLOR_GRAY);
    tui_draw_text(tui, 21, 13, fk_version(), FK_COLOR_CYAN);
    
    tui_draw_text(tui, 4, 19, "Press any key to exit...", FK_COLOR_GRAY);
    
    /* Render TUI */
    tui_render(tui);
    
    /* Wait for input */
    getchar();
    
    /* Cleanup */
    tui_destroy(tui);
    fk_shutdown();
    
    printf("\nTUI Demo complete!\n");
    
    return 0;
}
