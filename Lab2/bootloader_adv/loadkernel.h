#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001C
#define PM_WDOG 0x3F100024

extern int __self_size;

void copy_transfer();
void load_kernel();