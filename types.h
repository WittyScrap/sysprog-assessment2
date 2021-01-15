typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;
typedef unsigned char bool;
typedef uint pde_t;

enum {
	false,
	true,
};

#define MAX_BATCHED_OPS 50
#define MAX_BATCHED_DATA 10

typedef enum {
    BC_POINT,
    BC_LINE,
    BC_CLEAR,
    BC_RECT,
    BC_CIRCLE,
    BC_IMAGE,
    BC_EMPTYRECT,
    BC_EMPTYCRCL,
    // TODO: Add more primitives...
} batchedcall;

typedef struct {
    batchedcall type;
    int color;
    int data[MAX_BATCHED_DATA];
} batchedoperation;

typedef struct {
    batchedoperation ops[MAX_BATCHED_OPS];
    int count;
} batchedqueue;

typedef struct {
    int x;
    int y;
} point;

#define MAX_BITMAP_SIZE_BYTES 32*32

typedef struct {
    int height;
    int stride;
    int size;
    char data[MAX_BITMAP_SIZE_BYTES];
} bitmap;