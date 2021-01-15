struct stat;
struct rtcdate;

// system calls

int fork(void);
int exit(void) __attribute__((noreturn));
int wait(void);
int pipe(int*);
int write(int, const void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(char*, char**);
int open(const char*, int);
int mknod(const char*, short, short);
int unlink(const char*);
int fstat(int fd, struct stat*);
int link(const char*, const char*);
int mkdir(const char*);
int chdir(const char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);
int getch(void);
int setvideomode(int);
int plotpixel(int, int, int);
int plotline(int, int, int, int, int);
int clear(int);
int present(void);
int flush(batchedqueue*);
int loadbitmap(const char*, bitmap*);
int plotimage(bitmap* img, int x, int y);
// TODO: Declare your user APIs for your system calls.


// ulib.c

int stat(const char*, struct stat*);
char* strcpy(char*, const char*);
void *memmove(void*, const void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void printf(int, const char*, ...);
char* gets(char*, int max);
uint strlen(const char*);
void* memset(void*, int, uint);
void* malloc(uint);
void free(void*);
int atoi(const char*);


// graphics.c

void begingraphics();
void drawpoint(int x, int y, int c);
void drawline(int x0, int y0, int x1, int y1, int c);
void drawrect(int x, int y, int w, int h, int c);
void drawemptyrect(int x, int y, int w, int h, int c);
void drawcircle(int x, int y, int r, int c);
void drawpolygon(int count, int color, int x, int y, point vertices[]);
void drawimage(bitmap* img, int x, int y);
void drawemptycircle(int x, int y, int r, int c);
void clearcolor(int c);
void endgraphics();

// random.c

void seed(int);
uint random(void);
int randomrange(int lo, int hi);
