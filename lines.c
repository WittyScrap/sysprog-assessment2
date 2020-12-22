#include "types.h"
#include "user.h"

int main() {
    setvideomode(0x12);
    
    plotline(0,     0,      639,    480,    8);
    plotline(0,     60,     639,    420,    9);
    plotline(0,     120,    639,    360,    10);
    plotline(0,     180,    639,    300,    11);
    plotline(0,     240,    639,    240,    12);
    plotline(0,     300,    639,    180,    13);
    plotline(0,     360,    639,    120,    14);
    plotline(0,     420,    639,    60,     15);
    plotline(0,     480,    639,    0,      7);
    plotline(320,   0,      320,    480,    6);

    printf(1, "This was printed in mode 0x12...\n");

    getch();
    setvideomode(0x03);
    
    exit();
}