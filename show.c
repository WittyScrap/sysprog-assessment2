#include "types.h"
#include "user.h"

int main(int argc, char** argv) {
    if (argc <= 1) {
        printf(0, "You must specify an image file.\n");
        exit();
    }
    
    setvideomode(0x13);

    image img;
    loadbitmap(argv[1], &img);

    begingraphics();

    drawimage(img.id, 320 / 2 - img.width / 2, 200 / 2 - img.height / 2);

    endgraphics();
    present();

    getch();
    setvideomode(0x03);

    exit();
}