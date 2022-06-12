typedef unsigned int size_t;

int getpid();
size_t uart_read(char buf[], size_t size);
size_t uart_write(const char buf[], size_t size);
int exec(char *name, char *const argv[]);
int fork();
void exit(int status);

void kill(int pid);