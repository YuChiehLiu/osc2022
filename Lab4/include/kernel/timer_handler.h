void get_nowtime();
void L0_timer_handler(long cntpct_el0, long cntfrq_el0);
void add_timer(int sec, char* mes);
void L1_timer_handler(long cntpct_el0, long cntfrq_el0);
int is_empty();
void find_min();