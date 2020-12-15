// Memory layout

#define EXTMEM  0x100000            // Start of extended memory
#define PHYSTOP 0xE000000           // Top physical memory
#define DEVSPACE 0xFE000000         // Other devices are at high addresses

// Key addresses for address space layout (see kmap in vm.c for layout)
#define KERNBASE 0x80000000         // First kernel virtual address
#define KERNLINK (KERNBASE + EXTMEM)  // Address where kernel is linked

#define V2P(a) (((uint) (a)) - KERNBASE)
#define P2V(a) ((void *)(((char *) (a)) + KERNBASE))

#define V2P_WO(x) ((x) - KERNBASE)    // same as V2P, but without casts
#define P2V_WO(x) ((x) + KERNBASE)    // same as P2V, but without casts

#define VGA_0x03_MEMORY P2V(0xb8000)
#define VGA_0x03_WIDTH 80
#define VGA_0x03_HEIGHT 25

#define VGA_0x13_MEMORY P2V(0xA0000)
#define VGA_0x13_WIDTH 320
#define VGA_0x13_HEIGHT 200

#define VGA_0x03_OFFSET(x, y) ((y) * VGA_0x03_WIDTH + (x))
#define VGA_0x13_OFFSET(x, y) ((y) * VGA_0x13_WIDTH + (x))

#define VGA_0x03_MAXSIZE VGA_0x03_WIDTH * VGA_0x03_HEIGHT
#define VGA_0x13_MAXSIZE VGA_0x13_WIDTH * VGA_0x13_HEIGHT

#define VGA_0x03_MAXSIZE_BYTES VGA_0x03_WIDTH * VGA_0x03_HEIGHT * sizeof(uchar)
#define VGA_0x13_MAXSIZE_BYTES VGA_0x13_WIDTH * VGA_0x13_HEIGHT * sizeof(ushort)