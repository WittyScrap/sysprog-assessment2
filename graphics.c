#include "types.h"
#include "user.h"

// Batched operations
static Batch operations;
static int argstemp[10];

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
void addoperation(BatchedCall type, int c) {
    if (operations.count >= MAX_BATCHED_OPS) {
        flush(&operations);
        operations.count = 0;
    }

    BatchedOperation op;
    op.type = type;
    op.color = c;

    memmove(op.data, argstemp, sizeof(argstemp));

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
    argstemp[0] = x;
    argstemp[1] = y;

    addoperation(BC_POINT, c);
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
    argstemp[0] = x0;
    argstemp[1] = y0;
    argstemp[2] = x1;
    argstemp[3] = y1;

    addoperation(BC_LINE, c);
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
    argstemp[0] = x;
    argstemp[1] = y;
    argstemp[2] = w;
    argstemp[3] = h;

    addoperation(BC_RECT, c);
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
    argstemp[0] = x;
    argstemp[1] = y;
    argstemp[2] = r;
    
    addoperation(BC_CIRCLE, c);
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