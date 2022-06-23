#include "vfs.h" 

#define SECTOR_SIZE 512

/* Unsigned.  */
typedef unsigned char       uint8_t;
typedef unsigned short int  uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long int   uint64_t;

typedef struct partition_struct
{
    uint8_t     bootindicator;
    uint8_t     start_head;
    uint16_t    start_sector_and_cylinder;
    uint8_t     fs_type; // '0B':FAT32 ; '04':FAT16 ; '07':NTFS
    uint8_t     end_head;
    uint16_t    end_sector_and_cylinder;
    uint32_t    relative_sector;
    uint32_t    number_sectors;
}__attribute__((packed)) partition_t;

typedef struct MBR_struct
{
    uint8_t     bootstrap_code[446];
    partition_t partitiontable[4];
    uint8_t     signature[2];
}__attribute__((packed)) MBR_t;

typedef struct FAT32_struct
{
    uint8_t     jmp_boot[3];            /* boot strap short or near jump */
    uint8_t     oem_name[8];            /* OEM Name - can be used to special case partition manager volumes */
    uint16_t    bytes_per_sec;          /* number of bytes per logical sector, must be one of 512,1024,2048,4096 */
    uint8_t     secs_per_clus;          /* number of sectors per cluster, must be 1,2,4,8,16,32,61,128 */
    uint16_t    reserved_secs;          /* reserved sectors */
    uint8_t     number_fats;            /* number of FATs */
    uint16_t    number_rdentries;       /* number of root directory entries */
    uint16_t    number_secs;            /* number of sectors */
    uint8_t     media_des;              /* media descriptor type */
    uint16_t    secs_per_fat16;         /* logical sectors per FAT16 */
    uint16_t    secs_per_track;         /* sectors per track */
    uint16_t    number_heads;           /* number of heads */
    uint32_t    hidden_secs;            /* hidden sectors (unused) */
    uint32_t    total_sec;              /* number of sectors in filesystem (if sectors == 0) */
    uint32_t    secs_per_fat32;         /* logical sectors per FAT32 */
    uint16_t    flags;                  /* bit 8: fat mirroring, low 4: active fat */
    uint16_t    version;                /* major, minor filesystem version */
    uint32_t    root_cluster;           /* first cluster in root directory */
    uint16_t    info_sector;            /* filesystem info sector number in FAT32 reserved area*/
    uint16_t    backup_boot;            /* backup boot sector */
    uint8_t     reserved[12];           /* Unused */
    uint8_t     bios_number;            /* Physical drive number */
    uint8_t     state;                  /* undocumented, but used for mount state. */
    uint8_t     signature;              /* extended boot signature */
    uint32_t    vol_id;                 /* volume ID */
    uint8_t     vol_label[11];          /* volume label */
    uint8_t     fs_type[8];             /* file system type e.g. "FAT32    " */
}__attribute__((packed)) FAT32_t;

typedef struct FAT_directory_struct
{
    uint8_t     name[8];
    uint8_t     ext[3];
    uint8_t     attribute;
    uint8_t     reserved;               /*Reserved for use by Windows NT*/
    uint8_t     create_time_ms;
    uint16_t    create_time;
    uint16_t    create_date;
    uint16_t    last_acc_date;
    uint16_t    cluster_hi;            /*High word of this entry's first cluster number*/
    uint16_t    modify_time;           /*Time of last write*/
    uint16_t    modify_date;
    uint16_t    cluster_lo;            /* Low word of this entry's first cluster number. */
    uint32_t    file_size;
}__attribute__((packed)) FAT_dir_t;

typedef struct FAT_file_info
{
    FAT_dir_t               *dir;
    vnode_t                 *vnode;
    char                    name[13]; //include '.' & '\0'
    int                     size;
    uint64_t                data_sector;
    struct FAT_file_info    *next;
}FAT_file_info_t;

extern uint32_t fat_sector;
extern uint32_t data_sector;

int parse_mbr();
int init_FAT32();

int fat32_mount(filesystem_t* fs, mount_t* mount);

int fat32_lookup(struct vnode* dir_node, struct vnode** target, char* component_name);
int fat32_create(struct vnode* dir_node, struct vnode** target, char* component_name);
int fat32_mkdir(struct vnode* dir_node, struct vnode** target, char* component_name);

int fat32_write(struct file* file, void* buf, size_t len);
int fat32_read(struct file* file, void* buf, size_t len);
int fat32_open(struct vnode* file_node, struct file** target);
int fat32_close(struct file* file);

uint32_t get_next_free_cluster();
uint32_t clusterid_to_blockid(uint32_t cluster_id);