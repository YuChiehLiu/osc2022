#include "sdc_driver.h"
#include "printf.h"
#include "string.h"
#include "slub_sys.h"
#include "buddy_sys.h"
#include "vfs.h"
#include "tmpfs.h"
#include "fat32.h"


static FAT_file_info_t *FAT_list=NULL;
static FAT32_t metadata;

uint32_t fat_sector;
uint32_t data_sector;

int parse_mbr()
{
    MBR_t mbr;
    readblock(0, &mbr);

    if(mbr.signature[0]!=0x55 || mbr.signature[1]!=0xAA)
        return -1;
    
    readblock(mbr.partitiontable[0].relative_sector, (void*)&metadata);

    fat_sector = mbr.partitiontable[0].relative_sector + metadata.reserved_secs;

    data_sector = fat_sector +  metadata.number_fats * metadata.secs_per_fat32;

    return 0;
}

int init_FAT32()
{
    parse_mbr();

    char *buf = malloc(sizeof(char) * SECTOR_SIZE);
    readblock(data_sector, buf);

    FAT_dir_t *fat_dentry=(FAT_dir_t*)buf;
    FAT_file_info_t* tmp=NULL;
    FAT_list=NULL;

    for(int i=0 ; i<SECTOR_SIZE/sizeof(FAT_dir_t) ; i++)
    {
        if(strlen((char*)fat_dentry->name) == 0)
            break;

        FAT_file_info_t *fat_file = malloc(sizeof(FAT_file_info_t));
        fat_file->dir = fat_dentry;
        fat_file->vnode = NULL;

        int cnt = 0;
        for(int j=0 ; j<8 && fat_dentry->name[j] != ' ' ; j++)
            fat_file->name[cnt++] = fat_dentry->name[j];

        fat_file->name[cnt++] = '.';
        for(int j=0 ; j<3 && fat_dentry->ext[j] != ' ' ; j++)
            fat_file->name[cnt++] = fat_dentry->ext[j];

        fat_file->name[cnt] = '\0';

        fat_file->size=fat_dentry->file_size;
        
        uint32_t cluster_offset = fat_dentry->cluster_hi << 16;
        cluster_offset += fat_dentry->cluster_lo;
        cluster_offset -= metadata.root_cluster;

        fat_file->data_sector = data_sector + cluster_offset;
        fat_file->next = NULL;

        if(FAT_list==NULL)
        {
            FAT_list = fat_file;
            tmp = FAT_list;
        }
        else
        {
            tmp->next = fat_file;
            tmp=tmp->next;
        }

        fat_dentry++;
    }

    return 0;
}

int fat32_mount(filesystem_t* fs, mount_t* mount)
{
    mount->root->v_ops = malloc(sizeof(vnode_operations_t));
    mount->root->v_ops->lookup = fat32_lookup;
    mount->root->v_ops->create = fat32_create;
    mount->root->v_ops->mkdir = fat32_mkdir;

    mount->root->f_ops = malloc(sizeof(file_operations_t));
    mount->root->f_ops->write = fat32_write;
    mount->root->f_ops->read = fat32_read;
    mount->root->f_ops->open = fat32_open;
    mount->root->f_ops->close = fat32_close;

    init_FAT32();

    FAT_file_info_t *target_file=FAT_list;
    int i=0;
    while(target_file != NULL)
    {
        fat32_lookup(mount->root, &(mount->root->internal->entry[i]), target_file->name);
        i++;
        target_file = target_file->next;
    }

    return 0;
}


int fat32_lookup(struct vnode* dir_node, struct vnode** target, char* component_name)
{
    switch(tmpfs_lookup(dir_node, target, component_name))
    {
        case NOTFOUND:
            break;
        case NOTDIR:
            return NOTDIR;
        case EXISTED:
            return EXISTED;
    }
    
    FAT_file_info_t *target_file=FAT_list;
    while(target_file != NULL)
    {
        if(strequ(target_file->name, component_name))
            break;
        target_file=target_file->next;
    }

    if(target_file==NULL)
        return NOTFOUND;

    tmpfs_create(dir_node, target, component_name);
    (*target)->internal->size = target_file->size;
    (*target)->internal->data = (char*)target_file->data_sector;
    target_file->vnode = *target;

    return 0;
}

int fat32_create(struct vnode* dir_node, struct vnode** target, char* component_name)
{
    FAT_file_info_t* new_fat32_file = malloc(sizeof(FAT_file_info_t));

    strcpy(component_name, new_fat32_file->name);
    new_fat32_file->size = 0;

    uint32_t free_block = clusterid_to_blockid(get_next_free_cluster());
    new_fat32_file->data_sector = free_block;

    new_fat32_file->next = NULL;

    tmpfs_create(dir_node, target, component_name);
    (*target)->internal->size = new_fat32_file->size;
    (*target)->internal->data = (char*)new_fat32_file->data_sector;
    new_fat32_file->vnode = *target;


    char *buf = malloc(sizeof(char) * SECTOR_SIZE);
    readblock(data_sector, buf);

    FAT_dir_t *fat_dentry=(FAT_dir_t*)buf;

    for(int i=0 ; i<SECTOR_SIZE/sizeof(FAT_dir_t) ; i++)
    {
        if(strlen((char*)fat_dentry->name) == 0)
            break;
        fat_dentry++;
    }

    for(int i=0 ; i<8 ; i++)
    {
        fat_dentry->name[i]=' ';
        if(i<3)
            fat_dentry->ext[i]=' ';
    }

    for(int i=0 ; i<8 ; i++)
    {
        if(component_name[i]!='.')
            fat_dentry->name[i] = component_name[i];
        else
            break;
    }

    int flag = 0;
    int ext_cnt =0;
    for(int i=0 ; i<strlen(component_name) ; i++)
    {
        if(component_name[i]!='.' && flag==0)
            continue;
        else if(component_name[i]=='.')
            flag=1;
        else if(component_name[i]!='.' && flag==1)
        {
            fat_dentry->ext[ext_cnt] = component_name[i];
            ext_cnt++;
        }
    }

    fat_dentry->attribute = 0x20;
    fat_dentry->cluster_hi = (free_block + metadata.root_cluster - data_sector) >> 16;
    fat_dentry->cluster_lo = (free_block + metadata.root_cluster - data_sector) & 0xffff;

    new_fat32_file->dir = fat_dentry;

    writeblock(data_sector, buf);

    return 0;
}


int fat32_mkdir(struct vnode* dir_node, struct vnode** target, char* component_name){return 0;}



int fat32_write(struct file* file, void* buf, size_t len)
{
    int target_block = (uint64_t)(file->vnode->internal->data) + (file->f_pos/500);
    int offset = file->f_pos % 500;

    char _buf[SECTOR_SIZE];
    strncpy((char*)buf, _buf+offset, len);

    writeblock(target_block, _buf);

    char *__buf = malloc(sizeof(char) * SECTOR_SIZE);
    readblock(data_sector, __buf);

    FAT_dir_t *fat_dentry=(FAT_dir_t*)__buf;

    for(int i=0 ; i<SECTOR_SIZE/sizeof(FAT_dir_t) ; i++)
    {
        int cnt = 0;
        char filename[13];
        for(int j=0 ; j<8 && fat_dentry->name[j] != ' ' ; j++)
            filename[cnt++] = fat_dentry->name[j];

        filename[cnt++] = '.';
        for(int j=0 ; j<3 && fat_dentry->ext[j] != ' ' ; j++)
            filename[cnt++] = fat_dentry->ext[j];

        filename[cnt] = '\0';

        if(strequ(filename, file->vnode->internal->name))
            break;
        fat_dentry++;
    }

    fat_dentry->file_size += len;
    file->f_pos += len;

    writeblock(data_sector, __buf);

    uint32_t fat_table[(metadata.number_fats-1) * metadata.secs_per_fat32 * SECTOR_SIZE / sizeof(uint32_t)];
    for(int i=0 ; i<(metadata.number_fats-1) * metadata.secs_per_fat32 ; i++)
        readblock(fat_sector+i, fat_table+i*SECTOR_SIZE/sizeof(uint32_t));

    fat_table[target_block + metadata.root_cluster - data_sector] = 0xfffffff;

    for(int i=0 ; i<(metadata.number_fats-1) * metadata.secs_per_fat32 ; i++)
        writeblock(fat_sector+i, fat_table+i*SECTOR_SIZE/sizeof(uint32_t));


    return len;
}

int fat32_read(struct file* file, void* buf, size_t len)
{
    int target_block = (uint64_t)(file->vnode->internal->data) + (file->f_pos/500);
    int offset = file->f_pos % 500;

    size_t readable_size = file->vnode->internal->size - file->f_pos;
    size_t read_len = len <= readable_size ? len : readable_size;

    char _buf[SECTOR_SIZE];
    readblock(target_block, _buf);

    strncpy(_buf+offset, buf, read_len);

    return read_len;
}

int fat32_open(struct vnode* file_node, struct file** target)
{
    *target = malloc(sizeof(file_t));
    (*target)->vnode = file_node;
    (*target)->f_pos = 0;
    (*target)->f_ops = file_node->f_ops;
    return 0;
}

int fat32_close(struct file* file){return 0;}

uint32_t get_next_free_cluster()
{   
    uint32_t* fat_table = malloc((metadata.number_fats-1) * metadata.secs_per_fat32 * SECTOR_SIZE);
    for(int i=0 ; i<(metadata.number_fats-1) * metadata.secs_per_fat32 ; i++)
        readblock(fat_sector+i, fat_table+i*SECTOR_SIZE/sizeof(uint32_t));

    int i=0;
    int ret=0;
    while(1)
    {
        if(fat_table[i] <= 0x0fffffff && fat_table[i] >= 0x0ffffff8 && fat_table[i+1]==0)
            ret = i+1;

        if(i==(metadata.number_fats-1) * metadata.secs_per_fat32 * SECTOR_SIZE / sizeof(uint32_t) - 1)
            break;
        
        i++;
    }

    return ret;
}

uint32_t clusterid_to_blockid(uint32_t cluster_id)
{
    return cluster_id - metadata.root_cluster + data_sector;
}