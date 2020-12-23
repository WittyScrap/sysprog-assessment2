#define VGA_0x03_MEMORY P2V(0xb8000)
#define VGA_0x03_WIDTH 80
#define VGA_0x03_HEIGHT 25

#define VGA_0x13_MEMORY P2V(0xA0000)
#define VGA_0x13_WIDTH 320
#define VGA_0x13_HEIGHT 200

#define VGA_0x12_MEMORY consolevgabuffer()
#define VGA_0x12_WIDTH 640
#define VGA_0x12_HEIGHT 480

#define VGA_0x03_MAXSIZE (VGA_0x03_WIDTH * VGA_0x03_HEIGHT)
#define VGA_0x13_MAXSIZE (VGA_0x13_WIDTH * VGA_0x13_HEIGHT)
#define VGA_0x12_MAXSIZE (VGA_0x12_WIDTH * VGA_0x12_HEIGHT)

#define VGA_0x03_OFFSET(x, y) ((y) * VGA_0x03_WIDTH + (x))
#define VGA_0x13_OFFSET(x, y) ((y) * VGA_0x13_WIDTH + (x))
#define VGA_0x12_OFFSET(x, y) ((y) * VGA_0x12_WIDTH + (x))

#define VGA_0x03_MAXSIZE_BYTES (VGA_0x03_WIDTH * VGA_0x03_HEIGHT * sizeof(ushort))
#define VGA_0x13_MAXSIZE_BYTES (VGA_0x13_WIDTH * VGA_0x13_HEIGHT * sizeof(uchar))
#define VGA_0x12_MAXSIZE_BYTES (VGA_0x12_WIDTH * VGA_0x12_HEIGHT / 8)

#define VGA_0x12_R 2
#define VGA_0x12_G 1
#define VGA_0x12_B 0
#define VGA_0x12_L 3