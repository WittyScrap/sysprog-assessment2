#include "types.h"
#include "user.h"

const char* bonepuns[] = {
    "Where do you learn about bones?\nOsteoclasst.",
    "What do you call it when a skeleton is having a great time?\nAn osteoblast.",
    "Why does a skeleton always tell the truth?\nHe wants tibia honest.",
    "Why did the skeleton start a fight?\nHe had a bone to pick.",
    "What’s the coolest part of a skeleton?\nThe hip.",
    "What do you call a funny bone?\nA humerus.",
    "Why was the skeleton stupid?\nHe was a numskull.",
    "Why was the skeleton so lonely?\nHe had no body.",
    "Why are bones so calm?\nNothing gets under their skin.",
    "What did the osteopathic medicine doctor bring to the potluck?\nSpare ribs.",
    "Why do skeletons get sick on windy days?\nIt goes right through them.",
    "Where do you imprison a naughty skeleton?\nA rib cage.",
    "Why can’t a group of skeletons ever get anything done?\nIt’s a skeleton crew.",
};

const char* signatures[] = {
    "I have brain damage.",
    "This was a mistake.",
    "Why are you still using this.",
    "It's as much your fault as it is mine.",
    "Why do I exist.",
};

// Christ
int main() {
    seed(uptime());

    setvideomode(0x13);
    printf(0, "You're gonna have a time\n");

    bitmap sans;

    loadbitmap("sans_01.bmp", &sans);
    plotimage(&sans, 320 / 2 - 32, 100 - 64);

    loadbitmap("sans_02.bmp", &sans);
    plotimage(&sans, 320 / 2, 100 - 64);

    loadbitmap("sans_11.bmp", &sans);
    plotimage(&sans, 320 / 2 - 32, 100 - 32);
    
    loadbitmap("sans_12.bmp", &sans);
    plotimage(&sans, 320 / 2, 100 - 32);
    
    loadbitmap("sans_21.bmp", &sans);
    plotimage(&sans, 320 / 2 - 32, 100);
    
    loadbitmap("sans_22.bmp", &sans);
    plotimage(&sans, 320 / 2, 100);
    
    loadbitmap("sans_31.bmp", &sans);
    plotimage(&sans, 320 / 2 - 32, 100 + 32);
    
    loadbitmap("sans_32.bmp", &sans);
    plotimage(&sans, 320 / 2, 100 + 32);

    present();
    getch();

    setvideomode(0x03);
    printf(0, "You had a time\n");

    int i = randomrange(0, sizeof(bonepuns) / sizeof(char*) - 1);
    int c = randomrange(0, sizeof(signatures) / sizeof(char*) - 1);
    
    printf(0, "\nBy the way, %s Hehehe. %s\n", bonepuns[i], signatures[c]);

    exit();
}