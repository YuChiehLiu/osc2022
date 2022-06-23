#define RUN_QUEUE_SIZE 21 // circular queue need one more entry for is_rq_full()
#define TOTAL_THREAD 10
#define READY 1
#define ZOMBIE 2
#define FORKING 4

typedef struct context_struct context_t;
typedef struct thread_struct thread_t;
typedef struct trapframe_struct trapframe_t;
typedef void (*threadtask)();

struct context_struct
{
    unsigned long x19; // x19~x28
    unsigned long x20;
    unsigned long x21;
    unsigned long x22;
    unsigned long x23;
    unsigned long x24;
    unsigned long x25;
    unsigned long x26;
    unsigned long x27;
    unsigned long x28;
    unsigned long fp; // x29
    unsigned long lr; // x30
    unsigned long sp;
};

struct thread_struct
{
    context_t context;
    int id;
    int child_id;
    unsigned long status;
    threadtask task;
    unsigned long kstack_start;     // kernel stack base
    unsigned long ustack_start;     // user stack base
    unsigned long usrpgm_load_addr; // user program load address
    unsigned long trapframe;        // using "unsigned long" to keep trapframe address, instead of claiming a "trapframe_t*" to avoid "Data Abort"
    thread_t *next;
};

struct trapframe_struct
{
    unsigned long x[31];
    unsigned long spsr_el1;
    unsigned long elr_el1;
    unsigned long sp_el0;
};

extern thread_t kernel_thread;
extern thread_t *zombie_thread;
extern thread_t *rq_head;

/* asm code */
void thread_init(thread_t *thread_ptr);
void switch_to(thread_t *prev, thread_t *next);
thread_t *get_current(void);
void store_thread(context_t *thread_ptr);

/* c code */
void schedule();
thread_t *thread_create(threadtask task);
void task_wrapper();
void idle();
int add_rq(thread_t *thread);
thread_t *del_rq();
void kill_zombies();
void recycle_thread(thread_t *current);
void do_fork();
void create_child(thread_t *parent);