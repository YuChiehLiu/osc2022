#define MAX_POOL_SIZE 256
#define MIN_CHUNK_SIZE 16

typedef struct chunk chunk;

struct chunk
{
    int size;
    int addr;
    int val;
    chunk* next;
    chunk* previous;
};

chunk* newnode();
int is_pool_init(int msize);
int malloc(int size_req);
int free(int addr);
int roundup_size(int size);
int find_chunk(int msize);
int find_pool(int addr);
int is_returnable(int msize);