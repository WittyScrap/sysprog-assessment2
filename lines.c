#include "types.h"
#include "user.h"

int main() {
    setvideomode(0x13);
    
    plotline(0,     0,      319,    199,    8);
    plotline(0,     25,     319,    175,    9);
    plotline(0,     50,     319,    150,    10);
    plotline(0,     75,     319,    125,    11);
    plotline(0,     100,    319,    100,    12);
    plotline(0,     125,    319,    75,     13);
    plotline(0,     150,    319,    50,     14);
    plotline(0,     175,    319,    25,     15);
    plotline(0,     199,    319,    0,      7);
    plotline(160,   0,      160,    199,    6);

    printf(1, "This was printed in mode 0x13...\n");

    getch();
    setvideomode(0x03);
    
    exit();
}