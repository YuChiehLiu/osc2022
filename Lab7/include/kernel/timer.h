#define TIMER_QUEUE_SIZE 6

typedef struct timer_queue_Node tq;

struct timer_queue_Node
{
    long long second;
    char message[21];
};

extern tq node[TIMER_QUEUE_SIZE];
extern int timer_front;
extern int timer_back;

/* C code */
void get_nowtime();
void add_timer(int sec, char *mes);
int is_full();
int is_empty();
void find_min();
void delay(long seconds);

/* ASM code */
void core_timer_enable();
void core_timer_disable();
void set_switch_timer();