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
    // TODO: Add more primitives...
} BatchedCall;

typedef struct {
    BatchedCall type;
    int color;
    int data[MAX_BATCHED_DATA];
} BatchedOperation;

typedef struct {
    BatchedOperation ops[MAX_BATCHED_OPS];
    int count;
} Batch;

typedef struct {
    int x;
    int y;
} Vertex;