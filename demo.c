#include "types.h"
#include "user.h"

point star12h[11] = {
    { 548, 274 },
    { 566, 338 },
    { 620, 340 },
    { 576, 381 },
    { 592, 446 },
    { 548, 408 },
    { 504, 446 },
    { 520, 381 },
    { 476, 340 },
    { 530, 338 },
    { 548, 273 },
};

point star13h[11] = {
    { 274, 114 },
    { 283, 141 },
    { 310, 142 },
    { 288, 159 },
    { 296, 186 },
    { 274, 170 },
    { 252, 186 },
    { 260, 159 },
    { 238, 142 },
    { 265, 141 },
    { 274, 114 },
};

int main() {
    setvideomode(0x12);
    begingraphics();
    
    drawline(0,     0,      639,    480,    8);
    drawline(0,     60,     639,    420,    9);
    drawline(0,     120,    639,    360,    10);
    drawline(0,     180,    639,    300,    11);
    drawline(0,     240,    639,    240,    12);
    drawline(0,     300,    639,    180,    13);
    drawline(0,     360,    639,    120,    14);
    drawline(0,     420,    639,    60,     15);
    drawline(0,     480,    639,    0,      7);
    drawline(320,   0,      320,    480,    6);

    drawrect     (300,   200,    250,    150,    6);
    drawemptyrect(250,   100,    100,    200,    7);
    
    drawcircle(200, 240, 120, 12);
    drawpolygon(11, 15, 0, 0, star12h);

    printf(1, "This was the demo for mode 0x12...\n");

    endgraphics();
    present();

    getch();
    setvideomode(0x13);
    begingraphics();

    drawline(0,     0,      319,    200,    8);
    drawline(0,     25,     319,    175,    9);
    drawline(0,     50,     319,    150,    10);
    drawline(0,     75,     319,    125,    11);
    drawline(0,     100,    319,    100,    12);
    drawline(0,     125,    319,    75,     13);
    drawline(0,     150,    319,    50,     14);
    drawline(0,     175,    319,    25,     15);
    drawline(0,     200,    319,    0,      7);
    drawline(160,   0,      160,    200,    6);
    
    drawrect     (150,   85,     125,    62,     6);
    drawemptyrect(125,   40,     50,     85,     15);

    drawcircle(100, 100, 50, 12);
    drawpolygon(11, 15, 0, 0, star13h);
    
    printf(1, "This was the demo for mode 0x13...\n");
    
    endgraphics();
    present();

    getch();
    setvideomode(0x03);

    printf(1, "Demo concluded.\n");
    
    exit();
}