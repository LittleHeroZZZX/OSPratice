/*
 * @Author: zhouxin zzzx@hdu.edu.cn
 * @Date: 2022-11-25 14:59:28
 * @LastEditTime: 2022-11-25 15:56:16
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
    unsigned int count;
    // 空闲块起始盘块
    unsigned int block_index;

} free_block_list;
typedef struct FCB{
    char filename[8];
    char exname[3];
    // 0: directory, 1: file
    unsigned char attribute;
    unsigned short create_time;
    unsigned short create_date;
    unsigned short last_modify_date;
    unsigned short last_modify_time;
    unsigned long length;
}fcb;

typedef struct super_block
{
    unsigned long block_count;
    unsigned long free_block_count;
    unsigned long free_block_list_index;
    // 第一块逻辑盘块的物理盘块号
    unsigned long block_offset;
    free_block_list *free_block_list;
} super_block;


void start_sys(char *bak_file, super_block *sb);
void my_format(super_block *sb);
void my_revover();