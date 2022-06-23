#define MAX_ORDER 11 // the max order in my Buddy System
#define ALLOCATED -1 // The idth frame is already allocated, hence not allocable.
#define FREE 99 // The idth frame is free, but it belongs to a larger contiguous memory block. Hence, buddy system does not directly allocate it.
#define MIN_PAGE_SIZE 4096
#define TOTAL_PAGE_FRAME 245760

#define NULL (void*)0

typedef struct pf_entry pf_entry;
typedef struct reserved_memory reserved_memory;

struct pf_entry
{
    int index;
    int val;
    int allord; // allocated order
    unsigned long addr;
    pf_entry* next;
    pf_entry* previous;
};

struct reserved_memory
{
    int start;
    int end;
    char name[30];
};

extern pf_entry pf[TOTAL_PAGE_FRAME];
extern reserved_memory RMlist[100];
extern int RMindex;

void pf_init();
int config_pf(int size_req);
void free_pf(int index);
int find_free(int order);
void release_rmb(int order, int index);
int is_buddy_free(int order, int index);
void insert_fa(int order, int index);
void remove_fa(int order, int index);
int is_freeable(int index);

/* for starup allocation */
void memory_init();
void memory_reserve(int start, int end, char* name);
int is_include_RM(unsigned long start, unsigned long end);

/* for demo functions */
void demo();
void demo_opp();
void found_RM(int pf_index, int RM_index);