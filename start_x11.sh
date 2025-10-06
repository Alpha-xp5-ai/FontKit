#!/bin/bash
export DISPLAY=$(cat /etc/resolv.conf | grep nameserver | awk '{print $2}'):0
echo "DISPLAY set to: $DISPLAY"

# Quick test
cat > /tmp/test_x11.c << 'CODE'
#include <X11/Xlib.h>
#include <stdio.h>
int main() {
    Display *d = XOpenDisplay(NULL);
    if (d) {
        printf("✓ X11 working!\n");
        
        Window w = XCreateSimpleWindow(d, DefaultRootWindow(d), 
            100, 100, 600, 400, 2, 0, 0xFFFFFF);
        XStoreName(d, w, "Ubuntu Window!");
        XSelectInput(d, w, ExposureMask | KeyPressMask);
        XMapWindow(d, w);
        
        XEvent e;
        printf("✓ Window opened - press any key to close\n");
        while(1) {
            XNextEvent(d, &e);
            if (e.type == KeyPress) break;
            if (e.type == Expose) {
                GC gc = XCreateGC(d, w, 0, 0);
                XDrawString(d, w, gc, 50, 50, "Ubuntu Window Working!", 22);
            }
        }
        XCloseDisplay(d);
        return 0;
    }
    printf("✗ Cannot connect\n");
    return 1;
}
CODE

gcc /tmp/test_x11.c -o /tmp/test_x11 -lX11 && /tmp/test_x11
