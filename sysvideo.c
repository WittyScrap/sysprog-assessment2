#include "types.h"
#include "defs.h"
#include "memlayout.h"
#include "x86.h"
#include <stddef.h>

// Standby buffer (backup)
static uchar stbybuffer[VGA_0x03_MAXSIZE_BYTES] = { 0 };
uint cursor = 0;

// Back buffers
uchar backbuff12hR[VGA_0x12_MAXSIZE_BYTES] = { 0 };
uchar backbuff12hG[VGA_0x12_MAXSIZE_BYTES] = { 0 };
uchar backbuff12hB[VGA_0x12_MAXSIZE_BYTES] = { 0 };
uchar backbuff12hL[VGA_0x12_MAXSIZE_BYTES] = { 0 };
uchar backbuff13h[VGA_0x13_MAXSIZE_BYTES] = { 0 };

// Current VGA mode
extern uint currentvgamode;

/** --- Video mode handlers --- */

/**
 * Retrives the current console text mode buffer.
 * This will be a backup buffer if the selected VGA mode is not 0x03,
 * if it is, it'll simply return VGA_0x03_MEMORY.
 * 
 */
ushort* getcrtbuffer() {
    size_t mask = MASK(currentvgamode == 0x03);
    size_t ptr = (mask & (size_t)VGA_0x03_MEMORY) | (~mask & (size_t)stbybuffer);

    return (ushort*)ptr;
}

/**
 * Updates the cursor position.
 * This function may need to either update the position through
 * certain I/O ports or update the backed-up cursor location.
 * 
 * @param pos The new position of the cursor.
 */
void setcursorpos(uint pos) {
    if (currentvgamode == 0x03) {
        outb(CRTPORT, 14);
        outb(CRTPORT + 1, pos >> 8);
        outb(CRTPORT, 15);
        outb(CRTPORT + 1, pos);
    }

    cursor = pos;
}

/**
 * Retrieves hardware cursor position and updates 
 * cached reference to cursor position to match.
 * 
 */
void updatecursorpos() {
    outb(CRTPORT, 14);
    cursor = inb(CRTPORT + 1) << 8;
    outb(CRTPORT, 15);
    cursor |= inb(CRTPORT + 1);
}


/**
 * Sets a pixel at a given `x` and `y` coordinate to
 * the given color `c`.
 * 
 * @param x The x coordinate
 * @param y The y coordinate
 * @param c The color of the pixel
 * 
 */
void __setpixel0x13(int x, int y, int c) {
    uint offset = VGA_0x13_OFFSET(x, y);
    *((uchar*)VGA_0x13_MEMORY + MIN(offset, VGA_0x13_MAXSIZE)) = c;
}

/**
 * Sets a pixel at a given `x` and `y` coordinate to
 * the given color `c`.
 * 
 * @param x The x coordinate
 * @param y The y coordinate
 * @param c The color of the pixel
 * 
 */
void __setpixel0x12(int x, int y, int c) {
    uchar curr;
    uchar* mem;

    uint offset = VGA_0x12_OFFSET(x, y);
    uchar bit = 7 - (offset % 8);

    uchar mask = ~(1 << bit);

    offset /= 8;
    offset = MIN(offset, VGA_0x12_MAXSIZE_BYTES);
    
    uchar r = ((c & 0b0100) >> 2) << bit;
    uchar g = ((c & 0b0010) >> 1) << bit;
    uchar b = ((c & 0b0001) >> 0) << bit;
    uchar l = ((c & 0b1000) >> 3) << bit;

    mem = consoleselectplane(VGA_0x12_R);
    curr = *(mem + offset);
    curr = (curr & mask) | r;
    *(mem + offset) = curr;

    mem = consoleselectplane(VGA_0x12_G);
    curr = *(mem + offset);
    curr = (curr & mask) | g;
    *(mem + offset) = curr;

    mem = consoleselectplane(VGA_0x12_B);
    curr = *(mem + offset);
    curr = (curr & mask) | b;
    *(mem + offset) = curr;

    mem = consoleselectplane(VGA_0x12_L);
    curr = *(mem + offset);
    curr = (curr & mask) | l;
    *(mem + offset) = curr;
}

/**
 * Draws a line from `x0` and `y0` to `x1` and `y1` using
 * the given color `c`.
 * 
 * @param x0 The starting x coordinate
 * @param y0 The starting y coordinate
 * @param x1 The target x coordinate
 * @param y1 The target y coordinate
 * 
 */
void __drawline0x13(int x0, int y0, int x1, int y1, int c) {
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

    uint offset = 0;

    do {
        offset = VGA_0x13_OFFSET(x0, y0);
        *(vga + MIN(offset, VGA_0x13_MAXSIZE)) = c;

        int e2 = 2 * err;
        int c1 = MASK(e2 > -dy);
        int c2 = MASK(e2 < dx);
        
        err -= dy & c1;
        x0 += sx & c1;

        err += dx & c2;
        y0 += sy & c2;
    } while(x0 != x1 && y0 != y1);
}

/**
 * Draws a line from `x0` and `y0` to `x1` and `y1` using
 * the given color `c`.
 * 
 * @param x0 The starting x coordinate
 * @param y0 The starting y coordinate
 * @param x1 The target x coordinate
 * @param y1 The target y coordinate
 * 
 */
void __drawline0x12(int x0, int y0, int x1, int y1, int c) {
    int dx = x1 - x0;
    int dy = y1 - y0;

    int sx = SIGN(dx);
    int sy = SIGN(dy);

    dx = ABS(dx);
    dy = ABS(dy);

    int err = dx - dy;

    x1 += sx;
    y1 += sy;

    do {
        __setpixel0x12(x0, y0, c);

        int e2 = 2 * err;
        int c1 = MASK(e2 > -dy);
        int c2 = MASK(e2 < dx);
        
        err -= dy & c1;
        x0 += sx & c1;

        err += dx & c2;
        y0 += sy & c2;
    } while(x0 != x1 && y0 != y1);
}

/**
 * Clears the screen to a given color `c`.
 * 
 * @param c The color to clear the screen with.
 */
void __clearscr0x13(int c) {
    memset(VGA_0x13_MEMORY, c, VGA_0x13_MAXSIZE_BYTES);
}

/**
 * Clears the screen to a given color `c`.
 * 
 * @param c The color to clear the screen with.
 */
void __clearscr0x12(int c) {
    uchar r = (c & 0b0100) >> 2;
    uchar g = (c & 0b0010) >> 1;
    uchar b = (c & 0b0001) >> 0;
    uchar l = (c & 0b1000) >> 3;

    r = MASK(r);
    g = MASK(g);
    b = MASK(b);
    l = MASK(l);

    consolevgaplane(VGA_0x12_R);
    memset(VGA_0x12_MEMORY, r, VGA_0x12_MAXSIZE_BYTES);

    consolevgaplane(VGA_0x12_G);
    memset(VGA_0x12_MEMORY, g, VGA_0x12_MAXSIZE_BYTES);

    consolevgaplane(VGA_0x12_B);
    memset(VGA_0x12_MEMORY, b, VGA_0x12_MAXSIZE_BYTES);

    consolevgaplane(VGA_0x12_L);
    memset(VGA_0x12_MEMORY, l, VGA_0x12_MAXSIZE_BYTES);
}

/** --- Syscalls --- */

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

    // Backup old vga mode and reset current vga mode
    switch (mode)
    {
        case 0x13: {
            updatecursorpos();  // Backup cursor position
            memmove(stbybuffer, VGA_0x03_MEMORY, VGA_0x03_MAXSIZE_BYTES);  // Backup text mode memory
            int hr = consolevgamode(mode); // Switch to new vga mode
            __clearscr0x13(0); // Erase screen data

            return hr;
        }

        case 0x12: {
            updatecursorpos();  // Backup cursor position
            memmove(stbybuffer, VGA_0x03_MEMORY, VGA_0x03_MAXSIZE_BYTES);  // Backup text mode memory
            int hr = consolevgamode(mode); // Switch to new vga mode
            __clearscr0x12(0); // Erase screen data

            return hr;
        }

        case 0x03: {
            int hr = consolevgamode(mode); // Switch to text mode
            memmove(VGA_0x03_MEMORY, stbybuffer, VGA_0x03_MAXSIZE_BYTES); // Restore text mode backup
            setcursorpos(cursor); // Restore cursor

            return hr;
        }
    }

    return -1;
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

    switch (currentvgamode) {
        case 0x03:
            return -1;

        case 0x12: {
            __setpixel0x12(x, y, color);
            break;
        }

        case 0x13: {
            __setpixel0x13(x, y, color);
            break;
        }
    }

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

    switch (currentvgamode) {
        case 0x03:
            return -1;

        case 0x12: {
            __drawline0x12(x0, y0, x1, y1, color);
            break;
        }

        case 0x13: {
            __drawline0x13(x0, y0, x1, y1, color);
            break;
        }
    }

    return 0;
}

/**
 * Presents the stored back buffer for the
 * appropriate VGA mode into the front buffer.
 * 
 */
int sys_present(void) {
    switch (currentvgamode) {
        case 0x13: {
            // Trivial case, just copy back buffer into front buffer
            memmove(VGA_0x13_MEMORY, backbuff13h, VGA_0x13_MAXSIZE_BYTES);
        }

        case 0x12: {
            // Nontrivial case, select planes R through L and copy corresponding
            // back buffers into front buffers.
            consolevgaplane(VGA_0x12_R);
            memmove(VGA_0x12_MEMORY, backbuff12hR, VGA_0x12_MAXSIZE_BYTES);
            
            consolevgaplane(VGA_0x12_G);
            memmove(VGA_0x12_MEMORY, backbuff12hR, VGA_0x12_MAXSIZE_BYTES);

            consolevgaplane(VGA_0x12_B);
            memmove(VGA_0x12_MEMORY, backbuff12hR, VGA_0x12_MAXSIZE_BYTES);

            consolevgaplane(VGA_0x12_L);
            memmove(VGA_0x12_MEMORY, backbuff12hL, VGA_0x12_MAXSIZE_BYTES);
        }
    }
}