typedef unsigned int size_t;

int getpid();
size_t uart_read(char buf[], size_t size);
size_t uart_write(const char buf[], size_t size);
int exec(char *name, char *const argv[]);
int fork();
void exit(int status);
void kill(int pid);

int open(char *pathname, int flags);
int close(int fd);
long write(int fd, void *buf, unsigned long count);
long read(int fd, void *buf, unsigned long count);
int mkdir(char *pathname, unsigned mode);
int mount(char *src, char *target, char *filesystem, unsigned long flags, void *data);
int chdir(char *path);
