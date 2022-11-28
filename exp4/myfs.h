/*
 * @Author: zhouxin zzzx@hdu.edu.cn
 * @Date: 2022-11-25 14:59:28
 * @LastEditTime: 2022-11-25 21:33:32
 * @Software: VSCode
 */
#include "list.h"

#define BLOCK_SIZE 1024
#define SIZE 1024000
#define FILENAME_LEN 255


typedef struct free_block_list
{
    struct list_head list;
    // 空闲块数量
    size_t count;
    // 空闲块起始盘块
    size_t block_index;

} free_block_list;
typedef struct FCB{
    char filename[8];
    char exname[3];
    // 0: directory, 1: file
    unsigned char attribute;
    size_t create_time;
    size_t create_date;
    size_t last_modify_date;
    size_t last_modify_time;
    size_t length;
}fcb;

typedef struct super_block
{
    size_t block_count;
    size_t free_block_count;
    size_t free_block_list_index;
    //第零块逻辑盘块的物理盘地址
    size_t block_offset;

    // 文件系统在内存中的起始地址
    void *start_pos;
    free_block_list free_block_list;
} super_block;


void start_sys(char *bak_file, super_block **sb, int recreate);
void my_format(super_block **sb);
void revover(super_block *sb, char *bak_file);
void save(char *bak_file, void* start_pos, size_t size);
void* allocate_block(super_block *sb, size_t block_count);
