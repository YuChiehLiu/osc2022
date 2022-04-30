#define MAX_BUFFER_LEN 80
#define UNKNOWN -1
#define BACKSPACE 8
#define NEWLINE 10
#define LEGAL_INPUT 200

extern void main();
int distinguish(char c);
void exe_command(char in_c, int in_v, char* string, int* buffer_counter);
void exe_command2(int in_v, char *string, int *buffer_counter);
int check_format(char *string, char* commandname);

