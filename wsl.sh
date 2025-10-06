#!/bin/bash
# Quick X11 Window Test for WSL

echo "╔════════════════════════════════════════╗"
echo "║  Quick X11 Window Test for WSL        ║"
echo "╚════════════════════════════════════════╝"
echo ""

# Try to detect WSL version
if grep -qi microsoft /proc/version; then
    if grep -qi "WSL2" /proc/version; then
        echo "Detected: WSL2"
        WSL_VERSION=2
    else
        echo "Detected: WSL1"
        WSL_VERSION=1
    fi
else
    echo "Detected: Native Linux"
    WSL_VERSION=0
fi

echo ""

# Set DISPLAY based on WSL version
if [ $WSL_VERSION -eq 2 ]; then
    echo "Setting DISPLAY for WSL2..."
    WINDOWS_IP=$(cat /etc/resolv.conf | grep nameserver | awk '{print $2}')
    export DISPLAY=$WINDOWS_IP:0
    echo "export DISPLAY=$WINDOWS_IP:0" >> ~/.bashrc
    echo "DISPLAY=$DISPLAY"
elif [ $WSL_VERSION -eq 1 ]; then
    echo "Setting DISPLAY for WSL1..."
    export DISPLAY=:0
    echo "DISPLAY=:0"
else
    echo "Using existing DISPLAY setting..."
    echo "DISPLAY=${DISPLAY:-:0}"
fi

echo ""
echo "═══════════════════════════════════════"
echo "  STEP 1: Start X Server on Windows"
echo "═══════════════════════════════════════"
echo ""
echo "1. Download VcXsrv:"
echo "   https://sourceforge.net/projects/vcxsrv/"
echo ""
echo "2. Run XLaunch and configure:"
echo "   ✓ Multiple windows"
echo "   ✓ Display number: 0"  
echo "   ✓ Start no client"
echo "   ✓ Native opengl"
echo "   ✓ DISABLE ACCESS CONTROL ← IMPORTANT!"
echo ""
echo "3. Click 'Finish'"
echo ""
echo "Press Enter when VcXsrv is running..."
read

echo ""
echo "Testing X11 connection..."
sleep 2

# Compile simple test
echo "Compiling X11 test..."
cat > /tmp/x11test.c << 'EOF'
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
int main() {
    Display *d = XOpenDisplay(NULL);
    if (d) {
        printf("✓ X11 connection successful!\n");
        XCloseDisplay(d);
        return 0;
    }
    printf("✗ Cannot connect to X server\n");
    return 1;
}
EOF

gcc /tmp/x11test.c -o /tmp/x11test -lX11 2>/dev/null

if [ $? -ne 0 ]; then
    echo "Installing X11 development libraries..."
    sudo apt update && sudo apt install -y libx11-dev
    gcc /tmp/x11test.c -o /tmp/x11test -lX11
fi

echo ""
if /tmp/x11test; then
    echo ""
    echo "╔════════════════════════════════════════╗"
    echo "║  ✓ SUCCESS - X11 is working!          ║"
    echo "╚════════════════════════════════════════╝"
    echo ""
    echo "Now building window demo..."
    
    # Compile and run the demo
    gcc simple_x11.c -o simple_x11 -lX11 2>/dev/null
    
    if [ $? -eq 0 ]; then
        echo "✓ Demo compiled"
        echo "✓ Opening window..."
        ./simple_x11
    else
        echo "Creating demo..."
        cat > simple_x11.c << 'DEMOCODE'
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    Display *display = XOpenDisplay(NULL);
    if (!display) {
        printf("Failed to open display\n");
        return 1;
    }
    
    int screen = DefaultScreen(display);
    Window window = XCreateSimpleWindow(
        display, RootWindow(display, screen),
        100, 100, 800, 600, 4,
        BlackPixel(display, screen),
        WhitePixel(display, screen)
    );
    
    XStoreName(display, window, "Ubuntu Window - FontKit Demo");
    XSelectInput(display, window, ExposureMask | KeyPressMask);
    
    GC gc = XCreateGC(display, window, 0, NULL);
    XSetForeground(display, gc, BlackPixel(display, screen));
    
    XMapWindow(display, window);
    
    printf("✓ Window opened! Press any key in window to exit\n");
    
    XEvent event;
    int running = 1;
    while (running) {
        XNextEvent(display, &event);
        if (event.type == Expose) {
            XDrawString(display, window, gc, 50, 50, 
                "Ubuntu Window Working!", 22);
            XDrawString(display, window, gc, 50, 100,
                "Press any key to exit", 21);
        } else if (event.type == KeyPress) {
            running = 0;
        }
    }
    
    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return 0;
}
DEMOCODE
        gcc simple_x11.c -o simple_x11 -lX11
        ./simple_x11
    fi
    
else
    echo ""
    echo "╔════════════════════════════════════════╗"
    echo "║  ✗ X11 CONNECTION FAILED              ║"
    echo "╚════════════════════════════════════════╝"
    echo ""
    echo "Troubleshooting:"
    echo ""
    echo "1. Make sure VcXsrv is RUNNING (check system tray)"
    echo ""
    echo "2. Check Windows Firewall:"
    echo "   - Allow VcXsrv through firewall"
    echo ""  
    echo "3. Restart VcXsrv with correct settings:"
    echo "   - XLaunch → Multiple windows"
    echo "   - Display number: 0"
    echo "   - DISABLE ACCESS CONTROL ✓"
    echo ""
    echo "4. Try again: ./start_x11.sh"
fi

rm -f /tmp/x11test /tmp/x11test.c