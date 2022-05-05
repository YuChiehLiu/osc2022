#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001C
#define PM_WDOG 0x3F100024

void set(long addr, unsigned int value);
void command_help();
void command_hello();
void command_ls();
void command_cat();
void command_svc();
void command_time();
void command_reboot();
void command_not_found(char* string);
void command_setTimeout(char* string);
