#include "string.h"
#include "cpio.h"
#include "uart.h"
#include "slub_sys.h"
#include "buddy_sys.h"
#include "printf.h"
#include "vfs.h"
#include "tmpfs.h"

cpio_t* cpio_list;

void print_content(char* target_name)
{
    CPIO_NEWC_HEADER *target_addr = get_target_addr( (CPIO_NEWC_HEADER*)CPIO_START_ADDR, target_name );
    if( target_addr == 0 )
    {
        uart_puts("\rcat : ");
        uart_puts(target_name);
        uart_puts(": No such file or directory");
    }
    else
    {
        int namesize = get_size(target_addr, "name") -1;
        int filesize = get_size(target_addr, "file");
        int NUL_nums = 1;

        char* print_addr = (char*) (target_addr+1);
               
        while( (2+namesize+NUL_nums) % 4 != 0 )
            NUL_nums++;

        print_addr += (namesize+NUL_nums);
        
        uart_puts("\r");
        uart_myputs(print_addr, filesize);
    }  
}

void load_usrpgm(char* usrpgm_name, unsigned long load_addr)
{
    CPIO_NEWC_HEADER *target_addr = get_target_addr( (CPIO_NEWC_HEADER*)CPIO_START_ADDR, usrpgm_name );

    int namesize = get_size(target_addr, "name") -1;
    int filesize = get_size(target_addr, "file");
    int NUL_nums = 1;

    char* load_start_addr = (char*) load_addr;
    char* load_data_addr = (char*) (target_addr+1); // skip the content before filename

    while( (2+namesize+NUL_nums) % 4 != 0 )
            NUL_nums++;
    load_data_addr += (namesize+NUL_nums);

    for(int i=0 ; i<filesize ; i++)
    {
        *(load_start_addr+i) = *(load_data_addr+i);
    }
}

CPIO_NEWC_HEADER* get_target_addr(CPIO_NEWC_HEADER *current_file, char* target_name)
{
    int namesize = get_size(current_file, "name") -1;
    int filesize = get_size(current_file, "file");

    char *temp_addr = (char*) (current_file+1);

    char temp_name[20];

    for( int i=0 ; i<namesize ; i++)
    {
        temp_name[i] = temp_addr[i];
    }

    temp_name[namesize] = '\0';

    // No this file or directory
    if( strequ(temp_name, "TRAILER!!!") )
        return 0;
    // find the target
    else if( strequ(temp_name, target_name) )
        return current_file;
    // forward to next file
    else
    {
        char *next_file = temp_addr;
        int NUL_nums=1;

        //calculate the number of '\0' needed because of padding
        while( (2+namesize+NUL_nums) % 4 != 0 )
            NUL_nums++;

        next_file += (namesize+NUL_nums);
        NUL_nums=0;
        //calculate the number of '\0' needed because of padding 
        while( (filesize+NUL_nums) % 4 != 0 )
            NUL_nums++;        

        next_file += (filesize+NUL_nums);

        return get_target_addr( (CPIO_NEWC_HEADER*)next_file, target_name ); 
    }
    
    return 0;
}

int get_size(CPIO_NEWC_HEADER *root_addr, char* attr)
{
    char *temp_addr = (char*)root_addr;

    if ( strequ(attr, "name") )
        temp_addr += 94;
    else if( strequ(attr, "file") )
        temp_addr += 54;


    char size_string[9];
    for( int i=0 ; i<8 ; i++)
    {
        size_string[i] = temp_addr[i]; 
    }

    size_string[8] = '\0';
     

    //hexadecimal to decimal
    return hex2int(size_string);
    
}

/* for Lab7 basic4 */

int initramfs_mount(filesystem_t* fs, mount_t* mount)
{
    mount->root->v_ops = malloc(sizeof(vnode_operations_t));
    mount->root->v_ops->lookup = initramfs_lookup;
    mount->root->v_ops->create = initramfs_create;
    mount->root->v_ops->mkdir = initramfs_mkdir;

    mount->root->f_ops = malloc(sizeof(file_operations_t));
    mount->root->f_ops->write = initramfs_write;
    mount->root->f_ops->read = initramfs_read;
    mount->root->f_ops->open = initramfs_open;
    mount->root->f_ops->close = initramfs_close;

    init_cpio();

    cpio_t* tmp=cpio_list;
    int i=0;
    while(tmp!=NULL)
    {
        mount->root->internal->entry[i] = malloc(sizeof(vnode_t));
        mount->root->internal->entry[i]->mount = NULL;
        mount->root->internal->entry[i]->v_ops = mount->root->v_ops;
        mount->root->internal->entry[i]->f_ops = mount->root->f_ops;

        mount->root->internal->entry[i]->internal = malloc(sizeof(node_info_t));
        mount->root->internal->entry[i]->internal->name = malloc(sizeof(COMPONENT_NAME_MAX));
        strcpy(tmp->name ,mount->root->internal->entry[i]->internal->name);
        mount->root->internal->entry[i]->internal->type = tmp->type;
        mount->root->internal->entry[i]->internal->size = tmp->size;
        mount->root->internal->entry[i]->internal->data = tmp->data;

        mount->root->internal->size++;

        tmp = tmp->next;
        i++;
    }

    return 0;
}

void init_cpio()
{
    CPIO_NEWC_HEADER *current_file = (CPIO_NEWC_HEADER*)CPIO_START_ADDR;

    int namesize = get_size(current_file, "name") -1;
    int filesize = get_size(current_file, "file");

    char *temp_addr = (char*) (current_file+1);

    char temp_name[30];

    for( int i=0 ; i<namesize ; i++)
    {
        temp_name[i] = temp_addr[i];
    }

    temp_name[namesize] = '\0';

    if(strequ(temp_name, "TRAILER!!!"))
        return;
    
    cpio_list = malloc(sizeof(cpio_t));
    cpio_t* tmp=cpio_list;

    while(!strequ(temp_name, "TRAILER!!!"))
    {
        tmp->name = malloc(namesize+1);
        strcpy(temp_name, tmp->name);
        tmp->type = FILE;
        tmp->size = filesize;

        int NUL_nums = 1;
        char *next_file = (char*) (current_file+1);
       
        while( (2+namesize+NUL_nums) % 4 != 0 )
            NUL_nums++;

        next_file += (namesize+NUL_nums);
        tmp->data = next_file;

        NUL_nums=0;
        while( (filesize+NUL_nums) % 4 != 0 )
            NUL_nums++;        

        next_file += (filesize+NUL_nums);
        current_file = (CPIO_NEWC_HEADER*)next_file;

        namesize = get_size(current_file, "name") -1;
        filesize = get_size(current_file, "file");

        temp_addr = (char*) (current_file+1);

        for( int i=0 ; i<namesize ; i++)
            temp_name[i] = temp_addr[i];

        temp_name[namesize] = '\0';
        
        if(!strequ(temp_name, "TRAILER!!!"))
        {
            tmp->next = malloc(sizeof(cpio_t));
            tmp=tmp->next;         
        }
        else
            tmp->next = NULL;
    }
}

/* vnode operations : defined in tmpfs.h not in cpio.h */
int initramfs_lookup(struct vnode* dir_node, struct vnode** target, char* component_name)
{
    return tmpfs_lookup(dir_node, target, component_name);
}


int initramfs_create(struct vnode* dir_node, struct vnode** target, char* component_name)
{
    printf("/initramfs is read-only!!\n");
    return -1;
}

int initramfs_mkdir(struct vnode* dir_node, struct vnode** target, char* component_name)
{
    printf("/initramfs is read-only!!\n");
    return -1;
}

/* file operations */
int initramfs_write(struct file* file, void* buf, size_t len)
{
    printf("/initramfs is read-only!!\n");
    return -1;
}

int initramfs_read(struct file* file, void* buf, size_t len)
{
    return tmpfs_read(file, buf, len);
}

int initramfs_open(struct vnode* file_node, struct file** target)
{
    return tmpfs_open(file_node, target);
}

int initramfs_close(struct file* file)
{
    return tmpfs_close(file);
}