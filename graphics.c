#include "types.h"
#include "user.h"

// Batched operations
static batchedqueue operations;

/**
 * Begins a new graphics operation.
 * 
 */
void begingraphics() {
    operations.count = 0;
}

/**
 * Adds a new operation to the ops queue.
 * 
 */
void addoperation(batchedcall type, int c, ...) {
    if (operations.count >= MAX_BATCHED_OPS) {
        flush(&operations);
        operations.count = 0;
    }

    batchedoperation op;
    op.type = type;
    op.color = c;

    int* params = (int*)(void*)(&c + 1);
    memmove(op.data, params, sizeof(int) * MAX_BATCHED_DATA);

    operations.ops[operations.count] = op;
    operations.count += 1;
}

/**
 * Draws a point on a specific location designated
 * by `x` and `y`.
 * 
 * @param x X coordinate
 * @param y Y coordinate
 * @param c Color
 * 
 */
void drawpoint(int x, int y, int c) {
    addoperation(BC_POINT, c, x, y);
}

/**
 * Draws a line from a point designated by
 * `x0` and `y0` to a point designated by
 * `x1` and `y1` using the specified color `c`.
 * 
 * @param x0 Starting x coordinate
 * @param y0 Starting y coordinate
 * @param x1 Destination x coordinate
 * @param y1 Destination y coordinate
 * @param c  Color
 * 
 */
void drawline(int x0, int y0, int x1, int y1, int c) {
    addoperation(BC_LINE, c, x0, y0, x1, y1);
}

/**
 * Draws a rectangle located at a point designated by
 * `x` and `y` with a width of `w` and a height of `h`.
 * 
 * @param x Starting x coordinate
 * @param y Starting y coordinate
 * @param w Destination x coordinate
 * @param h Destination y coordinate
 * @param c Color
 * 
 */
void drawrect(int x, int y, int w, int h, int c) {
    addoperation(BC_RECT, c, x, y, w, h);
}

/**
 * Draws a filled circle with its center at `x`, `y`
 * of radius `r` with color `c`.
 * 
 * @param x The x coordinate of the center of the circle
 * @param y The y coordinate of the center of the circle
 * @param r The radius of the circle
 * @param c The color of the circle
 * 
 */
void drawcircle(int x, int y, int r, int c) {
    addoperation(BC_CIRCLE, c, x, y, r);
}

/**
 * Draws a polygon consisting of a series of connected lines.
 * 
 * @param count The number of vertices to draw.
 * @param color The color of the polygon.
 * @param x An horizontal offset for each vertex of the polygon.
 * @param y A vertical offset for each vertex of the polygon.
 * @param vertices The vertices list.
 * 
 */
void drawpolygon(int count, int color, int x, int y, point vertices[]) {
    point a = *vertices;
    
    for (int i = 1; i < count; i += 1) {
        point b = vertices[i];

        addoperation(BC_LINE, color, a.x + x, a.y + y, b.x + x, b.y + y);
        a = b;
    }
}

/**
 * Draws a bitmap stored in `img` at location `x`, `y`,
 * in mode 0x13. This function won't do anything in mode 0x12.
 * 
 */
void drawimage(bitmap* img, int x, int y) {
    addoperation(BC_IMAGE, 0, (int)(void*)img, x, y);
}

/**
 * Draws an empty, 1px thick rectangle frame at location
 * `x`, `y`, of size `w`, `h`, with color `c`.
 *
 */
void drawemptyrect(int x, int y, int w, int h, int c) {
    addoperation(BC_EMPTYRECT, c, x, y, w, h);
}

/**
 * Draws an empty, 1px thick circle at position `x`, `y`,
 * of radius `r`, using color `c`.
 * 
 */
void drawemptycircle(int x, int y, int r, int c) {
    addoperation(BC_EMPTYCRCL, c, x, y, r);
}

/**
 * Clears the screen to a specified color `c`.
 * 
 * @param c The color to clear the screen to.
 * 
 */
void clearcolor(int c) {
    addoperation(BC_CLEAR, c);
}

/**
 * Ends a graphics operation. If the initialized graphics
 * operation was in batched mode, this will flush any remaining
 * commands in the queue; otherwise, this function does nothing.
 * 
 */
void endgraphics() {
    flush(&operations);
}