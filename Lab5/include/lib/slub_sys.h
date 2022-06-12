#define MAX_POOL_SIZE 2048
#define MIN_CHUNK_SIZE 16

typedef struct chunk chunk;

struct chunk
{
    int size;
    unsigned long addr;
    int val;
    chunk* next;
    chunk* previous;
    chunk* nexthead; // point to next different page head in the same pool
};

chunk* newnode();
void* malloc(int size_req);
int is_pool_init(int msize);
int pool_extend(int msize);
int free(unsigned long addr);
int roundup_size(int size);
unsigned long find_chunk(int msize);
chunk* find_pool(unsigned long addr);
int is_returnable(chunk* head);