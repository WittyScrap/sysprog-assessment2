#include "types.h"
#include "defs.h"
#include "memlayout.h"

/** Syscalls --- */

int sys_getch(void) {
    return consoleget();
}

/**
 * System call for setting the video mode.
 * The following modes are supported:
 * 
 *   0x03: 80x25 text mode.
 *   0x12: 640x480x16 graphics mode.
 *   0x13: 320x200x256 graphics mode.
 */
int sys_setvideomode(void) {
    int mode;

    if (argint(0, &mode) < 0) {
        return -1;
    }

    int hr = consolevgamode(mode);

    // Backup old vga mode and reset current vga mode
    switch (mode)
    {
        case 0x13:
        memset(VGA_0x13_MEMORY, 0, VGA_0x13_WIDTH * VGA_0x13_HEIGHT * sizeof(uchar));
        break;

        case 0x03:
        memset(VGA_0x03_MEMORY, 0, VGA_0x03_WIDTH * VGA_0x03_HEIGHT * sizeof(ushort));
        break;
    }

    return hr;
}

/**
 * Plots a single pixel in any video mode.
 * Note that different video modes have different
 * limits.
 * 
 * Values are automatically clamped between 0 and the
 * maximum width/height combination.
 * 
 * @param x The x coordinate of the pixel.
 * @param y The y coordinate of the pixel.
 * @param color The color value of the pixel.
 */
int sys_setpixel(void) {
    int x;
    int y;
    int color;

    if (argint(0, &x) < 0 || argint(1, &y) < 0 || argint(2, &color)) {
        return -1;
    }

    uint offset = VGA_0x13_OFFSET(x, y);
    offset = offset & MASK(offset < VGA_0x13_MAXSIZE);
    uchar* vga = VGA_0x13_MEMORY + offset;
    *vga = color;

    return 0;
}

/**
 * Plots a line between any two points in any
 * video mode.
 * 
 * Values are automatically clamped between 0 and the
 * maximum width/height combination.
 * 
 * @param x0 Starting X coordinate
 * @param y0 Starting Y coordinate
 * @param x1 Ending X coordinate
 * @param y1 Ending Y coordinate
 * @param color The color of the line
 */
int sys_plotline(void) {
    int x0;
    int y0;
    int x1;
    int y1;
    int color;

    if (argint(0, &x0) < 0 || argint(1, &y0) < 0 || argint(2, &x1) < 0 || argint(3, &y1) < 0 || argint(4, &color) < 0) {
        return -1;
    }

    int dx = x1 - x0;
    int dy = y1 - y0;

    int sx = SIGN(dx);
    int sy = SIGN(dy);

    dx = ABS(dx);
    dy = ABS(dy);

    int err = dx - dy;
    uchar* vga = VGA_0x13_MEMORY;

    x1 += sx;
    y1 += sy;

    do {
        *(vga + VGA_0x13_OFFSET(x0, y0)) = color;

        int e2 = 2 * err;
        int c1 = MASK(e2 > -dy);
        int c2 = MASK(e2 < dx);
        
        err -= dy & c1;
        x0 += sx & c1;

        err += dx & c2;
        y0 += sy & c2;
    } while(x0 != x1 && y0 != y1);

    return 0;
}