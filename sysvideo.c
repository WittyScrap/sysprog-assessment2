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
    *(backbuff13h + MIN(offset, VGA_0x13_MAXSIZE)) = c;
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

    mem = backbuff12hR;
    curr = *(mem + offset);
    curr = (curr & mask) | r;
    *(mem + offset) = curr;

    mem = backbuff12hG;
    curr = *(mem + offset);
    curr = (curr & mask) | g;
    *(mem + offset) = curr;

    mem = backbuff12hB;
    curr = *(mem + offset);
    curr = (curr & mask) | b;
    *(mem + offset) = curr;

    mem = backbuff12hL;
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
        *(backbuff13h + MIN(offset, VGA_0x13_MAXSIZE)) = c;

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

        value = *(backbuff12hR + byte);
        value = (r & startmask) | (value & ~startmask);
        *(backbuff12hR + byte) = value;

        value = *(backbuff12hG + byte);
        value = (g & startmask) | (value & ~startmask);
        *(backbuff12hG + byte) = value;

        value = *(backbuff12hB + byte);
        value = (b & startmask) | (value & ~startmask);
        *(backbuff12hB + byte) = value;

        value = *(backbuff12hL + byte);
        value = (l & startmask) | (value & ~startmask);
        *(backbuff12hL + byte) = value;

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

        value = *(backbuff12hR + byte);
        value = (r & endmask) | (value & ~endmask);
        *(backbuff12hR + byte) = value;

        value = *(backbuff12hG + byte);
        value = (g & endmask) | (value & ~endmask);
        *(backbuff12hG + byte) = value;

        value = *(backbuff12hB + byte);
        value = (b & endmask) | (value & ~endmask);
        *(backbuff12hB + byte) = value;

        value = *(backbuff12hL + byte);
        value = (l & endmask) | (value & ~endmask);
        *(backbuff12hL + byte) = value;
        
        start = VGA_0x12_OFFSET(x, i);
        start = MIN(start, VGA_0x12_MAXSIZE);

        end = VGA_0x12_OFFSET(x + w, i);
        end = MIN(end, VGA_0x12_MAXSIZE);

        i += 1;
    } while(i <= y + h);
}

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

/** --- Function switchers --- */


/**
 * Here we store a pointer to all relevant
 * mode 12 functions...
 */
static const void(*mode12[])(int[static 10], int) = {
    [BC_POINT]  plotpixel0x12,
    [BC_LINE]   plotline0x12,
    [BC_RECT]   plotrect0x12,
    [BC_CLEAR]  clearscreen0x12,
    // TODO: Add any further primitive functions...
};

/**
 * Here we store a pointer to all relevant
 * mode 13 functions...
 */
static const void(*mode13[])(int[static 10], int) = {
    [BC_POINT]  plotpixel0x13,
    [BC_LINE]   plotline0x13,
    [BC_RECT]   plotrect0x13,
    [BC_CLEAR]  clearscreen0x13,
    // TODO: Add any further primitive functions...
};

/**
 * Strcutre used to switch between function sets.
 */
static const void(*(*switcher[]))(int[static 10], int) = {
    mode12,
    mode13,
};

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

    int index = currentvgamode - 0x12;
    switcher[index][BC_CLEAR](NULL, color);

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

    int index = currentvgamode - 0x12;
    switcher[index][BC_POINT](params, color);

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

    if (argint(0, &(params[0])) < 0 ||
        argint(1, &(params[1])) < 0 ||
        argint(2, &(params[2])) < 0 ||
        argint(3, &(params[3])) < 0 ||
        argint(4, &color) < 0) {
        return -1;
    }

    int index = currentvgamode - 0x12;
    switcher[index][BC_LINE](params, color);

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

    if (argptr(0, &charops, sizeof(BatchedCall)) < 0) {
        return -1;
    }

    Batch* bops = (Batch*)((void*)charops);
    
    int count = bops->count;
    BatchedOperation* ops = bops->ops;

    int index = currentvgamode - 0x12;

    for (int i = 0; i < count; i += 1) {
        switcher[index][ops[i].type](ops[i].data, ops[i].color);
    }

    return 0;
}