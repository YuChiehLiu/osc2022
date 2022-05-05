typedef struct timer_queue_Node tq;

struct timer_queue_Node
{
    long long second;
    char message[21];
};

extern tq node[10];
extern int front;
extern int back;

void get_nowtime();
void add_timer(int sec, char* mes);
int is_empty();
void find_min();