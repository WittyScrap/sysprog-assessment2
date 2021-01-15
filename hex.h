#define ROL(x, n) ((x << n) | (x >> (sizeof(uchar)*(8-n))))

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