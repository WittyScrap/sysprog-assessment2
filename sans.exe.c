#include "types.h"
#include "user.h"

// Christ
int main() {
    setvideomode(0x13);
    printf(0, "You're gonna have a time\n");

    {
        bitmap sans01;
        loadbitmap("sans_01.bmp", &sans01);
        plotimage(&sans01, 320 / 2 - 32, 100 - 64);
    }

    {
        bitmap sans02;
        loadbitmap("sans_02.bmp", &sans02);
        plotimage(&sans02, 320 / 2, 100 - 64);
    }

    {
        bitmap sans11;
        loadbitmap("sans_11.bmp", &sans11);
        plotimage(&sans11, 320 / 2 - 32, 100 - 32);
    }

    {
        bitmap sans12;
        loadbitmap("sans_12.bmp", &sans12);
        plotimage(&sans12, 320 / 2, 100 - 32);
    }

    {
        bitmap sans21;
        loadbitmap("sans_21.bmp", &sans21);
        plotimage(&sans21, 320 / 2 - 32, 100);
    }

    {
        bitmap sans22;
        loadbitmap("sans_22.bmp", &sans22);
        plotimage(&sans22, 320 / 2, 100);
    }

    {
        bitmap sans31;
        loadbitmap("sans_31.bmp", &sans31);
        plotimage(&sans31, 320 / 2 - 32, 100 + 32);
    }

    {
        bitmap sans32;
        loadbitmap("sans_32.bmp", &sans32);
        plotimage(&sans32, 320 / 2, 100 + 32);
    }

    present();
    getch();

    setvideomode(0x03);
    printf(0, "You had a time\n");


    exit();
}