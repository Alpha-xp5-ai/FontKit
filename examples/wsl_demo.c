/* Simple X11 Window Demo for WSL
 * Compile: gcc simple_x11.c -o simple_x11 -lX11
 * Run: ./simple_x11
 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    Display *display;
    Window window;
    XEvent event;
    int screen;
    
    printf("Attempting to connect to X server...\n");
    printf("DISPLAY=%s\n", getenv("DISPLAY") ? getenv("DISPLAY") : "not set");
    
    // Try multiple display options
    const char *displays[] = {":0", ":0.0", "localhost:0", NULL};
    
    for (int i = 0; displays[i] != NULL; i++) {
        printf("Trying DISPLAY=%s...\n", displays[i]);
        display = XOpenDisplay(displays[i]);
        if (display) {
            printf("✓ Connected with DISPLAY=%s\n", displays[i]);
            break;
        }
    }
    
    if (!display) {
        printf("\n❌ Failed to open X display!\n\n");
        printf("SOLUTIONS:\n");
        printf("1. Install VcXsrv on Windows:\n");
        printf("   https://sourceforge.net/projects/vcxsrv/\n\n");
        printf("2. Start VcXsrv with these settings:\n");
        printf("   - Multiple windows\n");
        printf("   - Display number: 0\n");
        printf("   - Start no client\n");
        printf("   - DISABLE ACCESS CONTROL ✓\n\n");
        printf("3. For WSL2, try:\n");
        printf("   export DISPLAY=$(cat /etc/resolv.conf | grep nameserver | awk '{print $2}'):0\n\n");
        return 1;
    }
    
    screen = DefaultScreen(display);
    
    // Create window
    window = XCreateSimpleWindow(
        display,
        RootWindow(display, screen),
        100, 100,   // x, y position
        800, 600,   // width, height
        4,          // border width
        BlackPixel(display, screen),
        WhitePixel(display, screen)
    );
    
    // Set window title
    XStoreName(display, window, "FontKit - Ubuntu Window Demo");
    
    // Select events
    XSelectInput(display, window, ExposureMask | KeyPressMask | ButtonPressMask);
    
    // Create graphics context
    GC gc = XCreateGC(display, window, 0, NULL);
    XSetForeground(display, gc, BlackPixel(display, screen));
    
    // Load font
    XFontStruct *font = XLoadQueryFont(display, "-*-helvetica-bold-r-*-*-24-*-*-*-*-*-*-*");
    if (!font) {
        font = XLoadQueryFont(display, "fixed");
    }
    if (font) {
        XSetFont(display, gc, font->fid);
    }
    
    // Map (show) window
    XMapWindow(display, window);
    XFlush(display);
    
    printf("\n✓ Window created successfully!\n");
    printf("✓ You should see an Ubuntu window now!\n");
    printf("✓ Press any key in the window to exit\n\n");
    
    // Event loop
    int running = 1;
    while (running) {
        XNextEvent(display, &event);
        
        switch (event.type) {
            case Expose:
                // Draw text
                XClearWindow(display, window);
                
                // Header
                XDrawString(display, window, gc, 50, 50, 
                    "FontKit Demo - Ubuntu Window!", 30);
                XDrawString(display, window, gc, 50, 100,
                    "============================", 28);
                
                // Info
                XDrawString(display, window, gc, 50, 150,
                    "✓ X11 Connection Working!", 25);
                XDrawString(display, window, gc, 50, 200,
                    "✓ Window Rendering Active", 25);
                
                // Instructions
                XSetForeground(display, gc, 0xFF0000); // Red
                XDrawString(display, window, gc, 50, 300,
                    "Press any key to exit", 21);
                
                XSetForeground(display, gc, BlackPixel(display, screen));
                XDrawString(display, window, gc, 50, 400,
                    "Next: Build FontKit with 'make wsl'", 35);
                XDrawString(display, window, gc, 50, 450,
                    "Then run: ./build/wsl_demo", 27);
                
                XFlush(display);
                break;
                
            case KeyPress:
                printf("Key pressed - exiting\n");
                running = 0;
                break;
                
            case ButtonPress:
                printf("Mouse clicked at: %d, %d\n", 
                    event.xbutton.x, event.xbutton.y);
                break;
        }
    }
    
    // Cleanup
    XFreeGC(display, gc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
    
    printf("✓ Window closed\n");
    return 0;
}