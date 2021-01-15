#include "types.h"
#include "stat.h"
#include "user.h"

#define ROL(x, n) ((x << n) | (x >> (sizeof(uchar)*(8-n))))

char buf[10];

void printhex(uchar value) {
    static const char hex_ascii[] = "0123456789ABCDEF";
    
    for (int i = 0; i < sizeof(value) * 2; i += 1) {
        uchar mask = 0x0F;
        value = ROL(value, 4);
        mask &= value;
        printf(1, "%c", hex_ascii[mask]);
    }

    printf(1, " ");
}

void hexdump(int fd, int off, int len) {
    int n;
    int l = len;

    {
        char skip[off];

        read(fd, skip, off);
        printf(1, "Reading from offset: %d\n", off);
    }

    while ((n = read(fd, buf, sizeof(buf))) > 0 && len > 0) {
        for (int i = 0; i < n && i < len; i += 1) {
            printhex(buf[i]);
        }
        printf(1, "\n");
        len -= n;
    }

    if (n < 0) {
        printf(1, "hexdump: read error\n");
        exit();
    }
    
    printf(1, "Read: %d\n", l);
}

int main(int argc, char *argv[]) {
    int fd, from, len;

    if (argc <= 3) {
        hexdump(0, 0, 0);
        exit();
    }

    fd = open(argv[1], 0);
    from = atoi(argv[2]);
    len = atoi(argv[3]);

    hexdump(fd, from, len);

    exit();
}
