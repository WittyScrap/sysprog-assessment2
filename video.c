#include "memlayout.h"
#include "types.h"

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
    memset(VGA_0x13_MEMORY, 0, VGA_0x13_MAXSIZE_BYTES);
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

}

/**
 * Clears the screen to a given color `c`.
 * 
 * @param c The color to clear the screen with.
 */
extern void __clearscr0x13(int c) {

}

/**
 * Clears the screen to a given color `c`.
 * 
 * @param c The color to clear the screen with.
 */
extern void __clearscr0x12(int c) {

}