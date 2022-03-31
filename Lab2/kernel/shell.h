#define MAX_BUFFER_LEN 14
#define UNKNOWN -1
#define BACKSPACE 8
#define NEWLINE 10
#define LEGAL_INPUT 200

void shell_start();
int distinguish(char c);
void exe_command(char in_c, int in_v, char* string, int* buffer_counter);


