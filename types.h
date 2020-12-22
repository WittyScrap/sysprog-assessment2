typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;
typedef unsigned char bool;
typedef uint pde_t;

enum {
	false,
	true,
};

enum BEGINMODE {
	BM_0x12_BATCHED,
	BM_0x12_IMMEDIATE,
	BM_0x13_BATCHED,
	BM_0x13_IMMEDIATE,
};