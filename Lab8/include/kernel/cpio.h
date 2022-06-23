#include "vfs.h" 

#define CPIO_START_ADDR 0x8000000
//#define CPIO_START_ADDR 0x20000000
#define C_MAGIC 0x070701
#define CPIO_END_SIGN "TRAILER!!!"
#define USRPGM_BASE 0x15000000
#define USRPGM_SIZE 0x100000 //1 mb

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

typedef struct cpio_node_struct
{
	char* name;
	int type;
	int size;
	char* data;
	struct cpio_node_struct* next;
}cpio_t;

void print_content(char *target_name);
CPIO_NEWC_HEADER *get_target_addr(CPIO_NEWC_HEADER *current_file, char *target_name);
int get_size(CPIO_NEWC_HEADER *root_addr, char *attr);
void load_usrpgm(char *usrpgm_name, unsigned long load_addr);

int initramfs_mount(filesystem_t* fs, mount_t* mount);
void init_cpio();


/* vnode operations */
int initramfs_lookup(struct vnode* dir_node, struct vnode** target, char* component_name);
int initramfs_create(struct vnode* dir_node, struct vnode** target, char* component_name);
int initramfs_mkdir(struct vnode* dir_node, struct vnode** target, char* component_name);

/* file operations */
int initramfs_write(struct file* file, void* buf, size_t len);
int initramfs_read(struct file* file, void* buf, size_t len);
int initramfs_open(struct vnode* file_node, struct file** target);
int initramfs_close(struct file* file);