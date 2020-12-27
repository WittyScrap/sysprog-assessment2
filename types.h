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

typedef enum {
    BC_POINT,
    BC_LINE,
    BC_CLEAR,
    BC_RECT,
    // TODO: Add remaining primitives...
} BatchedCall;

typedef struct {
    BatchedCall type;
    int color;
    int data[10];
} BatchedOperation;

typedef struct {
    BatchedOperation ops[MAX_BATCHED_OPS];
    int count;
} Batch;