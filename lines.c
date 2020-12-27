#include "types.h"
#include "user.h"

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

    drawrect(300, 200, 250, 150, 6);

    printf(1, "This was printed in mode 0x12...\n");

    endgraphics();
    present();

    getch();
    setvideomode(0x03);
    
    exit();
}