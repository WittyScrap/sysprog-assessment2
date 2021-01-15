#include "types.h"
#include "defs.h"
#include "memlayout.h"
#include "vga.h"
#include "x86.h"
#include "utils.h"
#include <stddef.h>

// Standby buffer (backup)
static uchar stbybuffer[VGA_0x03_MAXSIZE_BYTES] = { 0 };
uint cursor = 0;

// Current VGA mode
extern uint currentvgamode;

// Back buffers
static uchar backbuff12hR[VGA_0x12_MAXSIZE_BYTES] = { 0 };
static uchar backbuff12hG[VGA_0x12_MAXSIZE_BYTES] = { 0 };
static uchar backbuff12hB[VGA_0x12_MAXSIZE_BYTES] = { 0 };
static uchar backbuff12hL[VGA_0x12_MAXSIZE_BYTES] = { 0 };
static uchar backbuff13h [VGA_0x13_MAXSIZE_BYTES] = { 0 };

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

/** --- Graphics functions --- */

/**
 * Sets a pixel at a given `x` and `y` coordinate to
 * the given color `c`.
 * 
 * @param x The x coordinate
 * @param y The y coordinate
 * @param c The color of the pixel
 * 
 */
static void plotpixel0x13(int params[static 10], int c) {
    int x = params[0];
    int y = params[1];

    uint offset = VGA_0x13_OFFSET(x, y);
    backbuff13h[MIN(offset, VGA_0x13_MAXSIZE)] = c;
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
static void plotpixel0x12(int params[static 10], int c) {
    int x = params[0];
    int y = params[1];

    uchar curr;

    uint offset = VGA_0x12_OFFSET(x, y);
    uchar bit = 7 - (offset % 8);

    uchar mask = ~(1 << bit);

    offset /= 8;
    offset = MIN(offset, VGA_0x12_MAXSIZE_BYTES);
    
    uchar r = ((c & 0b0100) >> 2) << bit;
    uchar g = ((c & 0b0010) >> 1) << bit;
    uchar b = ((c & 0b0001) >> 0) << bit;
    uchar l = ((c & 0b1000) >> 3) << bit;

    curr = backbuff12hR[offset];
    curr = (curr & mask) | r;
    backbuff12hR[offset] = curr;

    curr = backbuff12hG[offset];
    curr = (curr & mask) | g;
    backbuff12hG[offset] = curr;

    curr = backbuff12hB[offset];
    curr = (curr & mask) | b;
    backbuff12hB[offset] = curr;

    curr = backbuff12hL[offset];
    curr = (curr & mask) | l;
    backbuff12hL[offset] = curr;
}

/**
 * Draws a line from `x0` and `y0` to `x1` and `y1` using
 * the given color `c`.
 * 
 * @param x0 The starting x coordinate
 * @param y0 The starting y coordinate
 * @param x1 The target x coordinate
 * @param y1 The target y coordinate
 * @param c  The color of the line
 * 
 */
static void plotline0x13(int params[static 10], int c) {
    int x0 = params[0];
    int y0 = params[1];
    int x1 = params[2];
    int y1 = params[3];

    int dx = x1 - x0;
    int dy = y1 - y0;

    int sx = SIGN(dx);
    int sy = SIGN(dy);

    dx = ABS(dx);
    dy = ABS(dy);

    int err = dx - dy;

    x1 += sx;
    y1 += sy;

    uint offset = 0;

    do {
        offset = VGA_0x13_OFFSET(x0, y0);
        backbuff13h[MIN(offset, VGA_0x13_MAXSIZE)] = c;

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
 * @param c  The color of the line
 * 
 */
static void plotline0x12(int params[static 10], int c) {
    int x0 = params[0];
    int y0 = params[1];
    int x1 = params[2];
    int y1 = params[3];

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
        params[0] = x0;
        params[1] = y0;

        plotpixel0x12(params, c);

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
 * Draws a rectangle at a specific location `x`, `y` and
 * with a specified size `w`, `h`, using a specified color `c`.
 * 
 * @param x The X coordinate of the top-left corner of the rectangle.
 * @param y The Y coordinate of the top-left corner of the rectangle.
 * @param w The width of the rectangle
 * @param h The height of the rectangle
 * @param c The color of the rectangle.
 * 
 */
static void plotrect0x13(int params[static 10], int c) {
    int x = params[0];
    int y = params[1];
    int w = params[2];
    int h = params[3];

    for (int i = y; i <= y + h; i += 1) {
        int offset = VGA_0x13_OFFSET(x, i);
        int count = MIN(offset + w, VGA_0x13_MAXSIZE) - offset;

        memset(backbuff13h + offset, c, count);
    }
}

/**
 * Draws a rectangle at a specific location `x`, `y` and
 * with a specified size `w`, `h`, using a specified color `c`.
 * 
 * @param x The X coordinate of the top-left corner of the rectangle.
 * @param y The Y coordinate of the top-left corner of the rectangle.
 * @param w The width of the rectangle
 * @param h The height of the rectangle
 * @param c The color of the rectangle.
 * 
 */
static void plotrect0x12(int params[static 10], int c) {
    int x = params[0];
    int y = params[1];
    int w = params[2];
    int h = params[3];

    VGA_UNWRAP_0x12_COLOR(c);

    h += 1;

    // Calculate the position of the "beginnig"
    // of the rectangle (the left edge)...
    int start = VGA_0x12_OFFSET(x, y);
    start = MIN(start, VGA_0x12_MAXSIZE);

    // Calculate the position of the "end"
    // of the rectangle (the right edge)...
    int end = VGA_0x12_OFFSET(x + w, y);
    end = MIN(end, VGA_0x12_MAXSIZE);
    
    // Find which bit represents the edge between the rect
    // and background
    uchar bit = start % 8;
    uchar startmask = ((bit - 1) | bit) | 1 << (bit - 1);   // Make a mask that has 1s where
                                                            // the rect is and 0s where the bg is.
                                                            // Here we add an extra bit to the left
                                                            // to include the X position itself in the
                                                            // rectangle.

    // Repeat for right edge
    bit = end % 8;
    uchar endmask = ~(bit - 1) | bit;

    int byte;
    int i = y;

    do {
        uchar value;

        /**
         * Fill in half bytes:
         * The mask represents which parts of the edge
         * byte are within the rectangle and which are outside of it.
         * 
         * For example, an edge byte that is 50% part of the rectangle
         * will have its mask byte look like this:
         * 
         * 00001111
         * 
         * We simply have to erase the background where the mask is 1 and
         * erase the replacement color's byte values where the mask is 0.
         * 
         * We do this by AND'ing the byte color channel with the mask and AND'ing
         * the existing background byte value with NOT mask.
         */

        // The actual starting byte is simply the starting position
        // divided by eight
        byte = start / 8;

        value = backbuff12hR[byte];
        value = (r & startmask) | (value & ~startmask);
        backbuff12hR[byte] = value;

        value = backbuff12hG[byte];
        value = (g & startmask) | (value & ~startmask);
        backbuff12hG[byte] = value;

        value = backbuff12hB[byte];
        value = (b & startmask) | (value & ~startmask);
        backbuff12hB[byte] = value;

        value = backbuff12hL[byte];
        value = (l & startmask) | (value & ~startmask);
        backbuff12hL[byte] = value;

        // Advance to next byte
        byte += 1;

        // From now on we can fill the full byte, until one byte
        // from the end (as that byte is likely also not going to be fully)
        // filled.
        int count = byte + w / 8;
        count = MIN(count, VGA_0x12_MAXSIZE_BYTES) - byte - 1;

        memset(backbuff12hR + byte, r, count);
        memset(backbuff12hG + byte, g, count);
        memset(backbuff12hB + byte, b, count);
        memset(backbuff12hL + byte, l, count);

        byte = end / 8;

        // Apply above solution to partially fill end byte.

        value = backbuff12hR[byte];
        value = (r & endmask) | (value & ~endmask);
        backbuff12hR[byte] = value;

        value = backbuff12hG[byte];
        value = (g & endmask) | (value & ~endmask);
        backbuff12hG[byte] = value;

        value = backbuff12hB[byte];
        value = (b & endmask) | (value & ~endmask);
        backbuff12hB[byte] = value;

        value = backbuff12hL[byte];
        value = (l & endmask) | (value & ~endmask);
        backbuff12hL[byte] = value;
        
        start = VGA_0x12_OFFSET(x, i);
        start = MIN(start, VGA_0x12_MAXSIZE);

        end = VGA_0x12_OFFSET(x + w, i);
        end = MIN(end, VGA_0x12_MAXSIZE);

        i += 1;
    } while(i <= y + h);
}

/**
 * Plots a circle at a given location and with a given
 * radius and color.
 * 
 * @param x The X coordinate of the circle's center
 * @param y The Y coordinate of the circle's center
 * @param c The color of the circle
 * 
 */
static void plotcircle0x13(int params[static 10], int c) {
    int cx = params[0];
    int cy = params[1];
    int r = params[2];

    for (int y = -r; y < r; y += 1) {
        for (int x = -r; x < r; x += 1) {
            int px = cx + x;
            int py = cy + y;

            int ic = x * x + y * y < r * r;

            uint offset = VGA_0x13_OFFSET(px, py);
            offset = MIN(offset, VGA_0x13_MAXSIZE_BYTES);

            uchar value = backbuff13h[offset];
            value = (value * !ic) + (c * ic);
            backbuff13h[offset] = value;
        }
    }
}

/**
 * Plots a circle at a given location and with a given
 * radius and color.
 * 
 * @param x The X coordinate of the circle's center
 * @param y The Y coordinate of the circle's center
 * @param c The color of the circle
 * 
 */
static void plotcircle0x12(int params[static 10], int c) {
    VGA_UNWRAP_0x12_COLOR(c);

    int cx = params[0];
    int cy = params[1];
    int cr = params[2];

    for (int y = -cr; y < cr; y += 1) {
        for (int x = -cr; x < cr; x += 1) {
            int px = cx + x;
            int py = cy + y;

            uchar ic = MASK(x * x + y * y < cr * cr);

            uint offset = VGA_0x12_OFFSET(px, py);
            offset = MIN(offset, VGA_0x12_MAXSIZE);
            uchar bit = 7 - (offset % 8);
            uchar mask = 1 << bit;
            offset /= 8;

            uchar value;

            value = backbuff12hR[offset];
            value = (value & ~mask) | ((value & ~ic) | (r & ic & mask));
            backbuff12hR[offset] = value;

            value = backbuff12hG[offset];
            value = (value & ~mask) | ((value & ~ic) | (g & ic & mask));
            backbuff12hG[offset] = value;

            value = backbuff12hB[offset];
            value = (value & ~mask) | ((value & ~ic) | (b & ic & mask));
            backbuff12hB[offset] = value;

            value = backbuff12hL[offset];
            value = (value & ~mask) | ((value & ~ic) | (l & ic & mask));
            backbuff12hL[offset] = value;
        }
    }
}

/**
 * Clears the screen to a given color `c`.
 * 
 * @param c The color to clear the screen with.
 * 
 */
static void clearscreen0x13(int params[static 10], int c) {
    memset(backbuff13h, c, VGA_0x13_MAXSIZE_BYTES);
}

/**
 * Clears the screen to a given color `c`.
 * 
 * @param c The color to clear the screen with.
 * 
 */
static void clearscreen0x12(int params[static 10], int c) {
    VGA_UNWRAP_0x12_COLOR(c);

    memset(backbuff12hR, r, VGA_0x12_MAXSIZE_BYTES);
    memset(backbuff12hG, g, VGA_0x12_MAXSIZE_BYTES);
    memset(backbuff12hB, b, VGA_0x12_MAXSIZE_BYTES);
    memset(backbuff12hL, l, VGA_0x12_MAXSIZE_BYTES);
}

#include "shex.h"

/**
 * Draws an image in mode 0x13.
 * 
 */
static void plotimage0x13(int params[static 10], int c) {
    const bitmap* bmp = (bitmap*)(void*)params[0];

    const int x = params[1];
    const int y = params[2];

    int stride = bmp->stride;
    int height = bmp->height;

    for (int i = 0; i < height; i += 1) {
        memmove(backbuff13h + VGA_0x13_OFFSET(x, i + y), bmp->data + i * stride, stride);
    }
}

/**
 * Draws an image in mode 0x12.
 * This function does nothing, as indexed bitmaps cannot
 * be drawn in mode 0x12.
 * 
 */
static void plotimage0x12(int params[static 10], int c) {
    cprintf("Note: indexed bitmaps cannot be drawn in mode 0x12...");
}

/**
 * Draws an empty rectangle in mode 0x13.
 * 
 */
static void plotemptyrect0x13(int params[static 10], int c) {
    int x = params[0];
    int y = params[1];
    int w = params[2];
    int h = params[3];

    memset(backbuff13h + VGA_0x13_OFFSET(x, y), c, w);
    memset(backbuff13h + VGA_0x13_OFFSET(x, y + h), c, w + 1);

    for (int end = y + h; y < end; y += 1) {
        *(backbuff13h + VGA_0x13_OFFSET(x, y)) = c;
        *(backbuff13h + VGA_0x13_OFFSET(x + w, y)) = c;
    }
}

/**
 * Draws an empty rectangle in mode 0x12.
 * 
 */
static void plotemptyrect0x12(int params[static 10], int c) {
    int x = params[0];
    int y = params[1];
    int w = params[2];
    int h = params[3];

    int p[2];

    for (int endx = x + w; x < endx; x += 1) {
        p[0] = x; p[1] = y;
        plotpixel0x12(p, c);
        
        p[1] = y + h;
        plotpixel0x12(p, c);
    }

    for (int endy = y + h; y < endy; y += 1) {
        p[0] = x - w; p[1] = y;
        plotpixel0x12(p, c);
        
        p[0] = x;
        plotpixel0x12(p, c);
    }
}

/**
 * Plots an empty, 1px thick circle at a given
 * location and radius.
 * 
 * Midpoint circle algorithm by https://rosettacode.org/wiki/Bitmap/Midpoint_circle_algorithm#C
 * 
 */
static void plotemptycircle0x13(int params[static 10], int c) {
    int x0 = params[0];
    int y0 = params[1];
    int r = params[2];
    
    int f = 1 - r;
    int ddF_x = 0;
    int ddF_y = -2 * r;
    int x = 0;
    int y = r;

    *(backbuff13h + VGA_0x13_OFFSET(x0, y0 + r)) = c;
    *(backbuff13h + VGA_0x13_OFFSET(x0, y0 - r)) = c;
    *(backbuff13h + VGA_0x13_OFFSET(x0 + r, y0)) = c;
    *(backbuff13h + VGA_0x13_OFFSET(x0 - r, y0)) = c;

    while (x < y) {
        if (f >= 0) {
            y -= 1;
            ddF_y += 2;
            f += ddF_y;
        }

        x += 1;
        ddF_x += 2;
        f += ddF_x + 1;

        *(backbuff13h + VGA_0x13_OFFSET(x0 + x, y0 + y)) = c;
        *(backbuff13h + VGA_0x13_OFFSET(x0 - x, y0 + y)) = c;
        *(backbuff13h + VGA_0x13_OFFSET(x0 + x, y0 - y)) = c;
        *(backbuff13h + VGA_0x13_OFFSET(x0 - x, y0 - y)) = c;
        *(backbuff13h + VGA_0x13_OFFSET(x0 + y, y0 + x)) = c;
        *(backbuff13h + VGA_0x13_OFFSET(x0 - y, y0 + x)) = c;
        *(backbuff13h + VGA_0x13_OFFSET(x0 + y, y0 - x)) = c;
        *(backbuff13h + VGA_0x13_OFFSET(x0 - y, y0 - x)) = c;
    }
}


/**
 * Plots an empty, 1px thick circle at a given
 * location and radius.
 * 
 * Midpoint circle algorithm by https://rosettacode.org/wiki/Bitmap/Midpoint_circle_algorithm#C
 * 
 */
static void plotemptycircle0x12(int params[static 10], int c) {
    int x0 = params[0];
    int y0 = params[1];
    int r = params[2];
    
    int f = 1 - r;
    int ddF_x = 0;
    int ddF_y = -2 * r;
    int x = 0;
    int y = r;
    
    params[0] = x0;
    params[1] = y0 + r;

    plotpixel0x12(params, c);

    params[0] = x0;
    params[1] = y0 - r;

    plotpixel0x12(params, c);

    params[0] = x0 + r;
    params[1] = y0;

    plotpixel0x12(params, c);

    params[0] = x0 - r;
    params[1] = y0;

    plotpixel0x12(params, c);

    while (x < y) {
        if (f >= 0) {
            y -= 1;
            ddF_y += 2;
            f += ddF_y;
        }

        x += 1;
        ddF_x += 2;
        f += ddF_x + 1;

        params[0] = x0 + x;
        params[1] = y0 + y;
        
        plotpixel0x12(params, c);

        params[0] = x0 - x;
        params[1] = y0 + y;

        plotpixel0x12(params, c);

        params[0] = x0 + x;
        params[1] = y0 - y;

        plotpixel0x12(params, c);

        params[0] = x0 - x;
        params[1] = y0 - y;

        plotpixel0x12(params, c);

        params[0] = x0 + y;
        params[1] = y0 + x;

        plotpixel0x12(params, c);
        
        params[0] = x0 - y;
        params[1] = y0 + x;

        plotpixel0x12(params, c);
        
        params[0] = x0 + y;
        params[1] = y0 - x;

        plotpixel0x12(params, c);

        params[0] = x0 - y;
        params[1] = y0 - x;

        plotpixel0x12(params, c);
    }
}

/** --- Function switchers --- */


/**
 * Here we store a pointer to all relevant
 * mode 12 functions...
 */
static const void(*mode12[])(int[static 10], int) = {
    [BC_CLEAR]      clearscreen0x12,
    [BC_POINT]      plotpixel0x12,
    [BC_LINE]       plotline0x12,
    [BC_RECT]       plotrect0x12,
    [BC_CIRCLE]     plotcircle0x12,
    [BC_IMAGE]      plotimage0x12,
    [BC_EMPTYRECT]  plotemptyrect0x12,
    [BC_EMPTYCRCL]  plotemptycircle0x12,
    // TODO: Add any further primitive functions...
};

/**
 * Here we store a pointer to all relevant
 * mode 13 functions...
 */
static const void(*mode13[])(int[static 10], int) = {
    [BC_CLEAR]      clearscreen0x13,
    [BC_POINT]      plotpixel0x13,
    [BC_LINE]       plotline0x13,
    [BC_RECT]       plotrect0x13,
    [BC_CIRCLE]     plotcircle0x13,
    [BC_IMAGE]      plotimage0x13,
    [BC_EMPTYRECT]  plotemptyrect0x13,
    [BC_EMPTYCRCL]  plotemptycircle0x13,
    // TODO: Add any further primitive functions...
};

#define FUNCTIONSET(mode) ((void(**)(int[static 10], int))(((uint)mode12 * ((mode) == 0x12)) + ((uint)mode13 * ((mode) == 0x13))))

/** --- Utilities --- */

/**
 * Stores the entirety of text memory into
 * a standby buffer.
 * 
 */
void backuptextmem() {
    if (currentvgamode == 0x03) {
        updatecursorpos();  // Backup cursor position
        memmove(stbybuffer, VGA_0x03_MEMORY, VGA_0x03_MAXSIZE_BYTES);  // Backup text mode memory
    }
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
    switch (mode) {
        case 0x13: {
            backuptextmem();
            int hr = consolevgamode(mode); // Switch to new vga mode
            clearscreen0x13(NULL, 0); // Erase screen data

            return hr;
        }

        case 0x12: {
            backuptextmem();
            int hr = consolevgamode(mode); // Switch to new vga mode
            clearscreen0x12(NULL, 0); // Erase screen data

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
 * Clears the screen to a given color.
 * 
 * Note that this operation affects the back buffer. For this
 * to be reflected to the screen, present() must be called.
 * 
 * @param color The color to use to clear the screen.
 * 
 */
int sys_clear(void) {
    int color;

    if (argint(0, &color) < 0) {
        return -1;
    }

    FUNCTIONSET(currentvgamode)[BC_CLEAR](NULL, color);

    return 0;
}

/**
 * Plots a single pixel in any video mode.
 * Note that different video modes have different
 * limits.
 * 
 * Values are automatically clamped between 0 and the
 * maximum width/height combination.
 * 
 * This is a system call, and will be executed immediately.
 * If batching is used, prefer draw<x> functions over plot<x> functions.
 * 
 * Note that this operation affects the back buffer. For this
 * to be reflected to the screen, present() must be called.
 * 
 * @param x The x coordinate of the pixel.
 * @param y The y coordinate of the pixel.
 * @param color The color value of the pixel.
 */
int sys_plotpixel(void) {
    int params[2];
    int color;

    if (argint(0, &(params[0])) < 0 || argint(1, &(params[1])) < 0 || argint(2, &color)) {
        return -1;
    }

    FUNCTIONSET(currentvgamode)[BC_POINT](params, color);

    return 0;
}

/**
 * Plots a line between any two points in any
 * video mode.
 * 
 * Values are automatically clamped between 0 and the
 * maximum width/height combination.
 * 
 * This is a system call, and will be executed immediately.
 * If batching is used, prefer draw<x> functions over plot<x> functions.
 * 
 * Note that this operation affects the back buffer. For this
 * to be reflected to the screen, present() must be called.
 * 
 * @param x0 Starting X coordinate
 * @param y0 Starting Y coordinate
 * @param x1 Ending X coordinate
 * @param y1 Ending Y coordinate
 * @param color The color of the line
 */
int sys_plotline(void) {
    int params[4];
    int color;

    if (argint(0, &(params[0])) < 0 || argint(1, &(params[1])) < 0 ||
        argint(2, &(params[2])) < 0 || argint(3, &(params[3])) < 0 ||
        argint(4, &color) < 0) {
        return -1;
    }

    FUNCTIONSET(currentvgamode)[BC_LINE](params, color);

    return 0;
}

/**
 * Presents the back buffer to the screen front buffer.
 * This function must be called in order for any draw operations
 * to be visible.
 * 
 */
int sys_present(void) {
    switch (currentvgamode) {
        case 0x13: {
            // Trivial case, bitblit back buffer to VGA addr
            memmove(VGA_0x13_MEMORY, backbuff13h, VGA_0x13_MAXSIZE_BYTES);
            break;
        }

        case 0x12: {
            // Nontrivial case, bitblit each back buffer into each VGA address plane
            memmove(consoleselectplane(VGA_0x12_R), backbuff12hR, VGA_0x12_MAXSIZE_BYTES);
            memmove(consoleselectplane(VGA_0x12_G), backbuff12hG, VGA_0x12_MAXSIZE_BYTES);
            memmove(consoleselectplane(VGA_0x12_B), backbuff12hB, VGA_0x12_MAXSIZE_BYTES);
            memmove(consoleselectplane(VGA_0x12_L), backbuff12hL, VGA_0x12_MAXSIZE_BYTES);
            break;
        }

        default:
            return -1;
    }

    return 0;
}

/**
 * Flushes a queue of batched draw operations.
 * 
 * @param batch The batched operations queue.
 * 
 */
int sys_flush(void) {
    char* charops;

    if (argptr(0, &charops, sizeof(batchedcall)) < 0) {
        return -1;
    }

    batchedqueue* bops = (batchedqueue*)((void*)charops);
    
    int count = bops->count;
    batchedoperation* ops = bops->ops;

    // This will yield undefined results in modes other than 12 or 13
    void(**set)(int[static 10], int) = FUNCTIONSET(currentvgamode); // Find the appropriate functions set (for mode 12 or 13)

    for (int i = 0; i < count; i += 1) {
        // From the set, pick the function for the primitive we are about to draw
        // by using the batchedcall type stored in the current batchedoperation, then call it using
        // the operation's argument data and the stored color.
        set[ops[i].type](ops[i].data, ops[i].color);
    }

    return 0;
}

/**
 * Bitmap offsets
 * 
 */
enum
{
    BMP_WIDTH = 0x12,
    BMP_HEIGHT = 0x16,
    BMP_OFFSET = 0x0A,
};

/**
 * Loads a bitmap stored in `filename`.
 * 
 */
int sys_loadbitmap() {
    char* filename;
    bitmap* img;
    struct inode *ip;

    if (argptr(0, &filename, sizeof(char*)) < 0 || argptr(1, (char**)&img, sizeof(bitmap*)) < 0) {
        return -1;
    }

    begin_op();

    if ((ip = namei(filename)) == 0) {
        end_op();
        cprintf("File not found: %s", filename);
        return -1;
    }

    ilock(ip);

    // Read in bitmap width and height
    readi(ip, (char*)&img->stride, BMP_WIDTH, sizeof(ushort));
    readi(ip, (char*)&img->height, BMP_HEIGHT, sizeof(ushort));

    cprintf("Loaded image of width: %d, height: %d\n", img->stride, img->height);

    img->size = img->stride * img->height;
    
    int dataOffset = 0;

    // Read data offset to know where to start reading image
    // data from
    readi(ip, (char*)&dataOffset, BMP_OFFSET, sizeof(ushort));

    // Now read bitmap data into temporary buffer
    char data[img->size];
    readi(ip, data, dataOffset, img->size);
    
    end_op();
    iunlockput(ip);

    // Cache to skip one indirection
    int stride = img->stride;

    for (int y = img->height - 1, pos = 0; y >= 0; y -= 1, pos += stride) {
        memmove(img->data + pos, data + y * stride, stride);
    }

    return 0;
}

/**
 * Draws an image in mode 0x13.
 * This function does nothing in mode 0x12, as
 * indexed images cannot be mapped to mode 12 colors
 * directly.
 * 
 */
int sys_plotimage(void) {
    bitmap* img;
    int x;
    int y;
    int color;

    if (argptr(0, (char**)&img, sizeof(bitmap*)) < 0 ||
        argint(1, &x) < 0 ||
        argint(2, &y) < 0 ||
        argint(1, &color) < 0) {
        return -1;
    }

    int params[10];
    params[0] = (int)(void*)img;
    params[1] = x;
    params[2] = y;

    FUNCTIONSET(currentvgamode)[BC_IMAGE](params, color);

    return 0;
}