#include "types.h"
#include "user.h"

int main() {
    setvideomode(0x13);

    image sans;
    loadbitmap("sans.bmp", &sans);

    begingraphics();

    printf(0, "You're gonna have a time");
    drawimage(sans.id, 0, 0);

    endgraphics();
    present();

    exit();
}