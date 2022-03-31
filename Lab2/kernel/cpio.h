#define CPIO_START_ADDR 0x8000000 // for QEMU 
// #define CPIO_START_ADDR 0x20000000 // for Raspi3
#define C_MAGIC 0x070701
#define CPIO_END_SIGN "TRAILER!!!"

typedef struct cpio_newc_header CPIO_NEWC_HEADER;

struct cpio_newc_header
{
	char c_magic[6];
	char c_ino[8];
	char c_mode[8];
	char c_uid[8];
	char c_gid[8];
	char c_nlink[8];
	char c_mtime[8];
	char c_filesize[8];
	char c_devmajor[8];
	char c_devminor[8];
	char c_rdevmajor[8];
	char c_rdevminor[8];
	char c_namesize[8];
	char c_check[8];
};

void print_content(char* target_name);
CPIO_NEWC_HEADER* get_target_addr(CPIO_NEWC_HEADER *current_file, char* target_name);
int get_size(CPIO_NEWC_HEADER *root_addr, char* attr);