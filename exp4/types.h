//
// Created by littleherozzzx on 02/12/2022.
// 存放公共数据结构和宏定义常量
//

#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include <time.h>
#include "list.h"


#define BLOCK_SIZE 1024
#define SIZE 1024000
#define FILENAME_LEN 255
#define DIR_MAX_COUNT ((BLOCK_SIZE-sizeof(size_t))/sizeof(inode))

#define ORDINARY_FILE 0
#define DIRECTORY 1


typedef struct free_block_list
{
    struct list_head list;
    // 空闲块数量
    size_t count;
    // 空闲块起始盘块
    size_t block_index;

} free_block_list;
typedef struct FCB{
    char filename[256];
    // 0: directory, 1: file
    unsigned char attribute;
    struct tm create_time;
    struct tm last_modify_time;
    size_t length;
}fcb;
typedef struct inode{
    char filename[255];
    size_t length;
    size_t inode_index;
    // 0: directory, 1: file
    unsigned char attribute;
} inode;
typedef struct dir
{
    size_t file_count;
    inode files[DIR_MAX_COUNT];
};
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
    inode *root;
} super_block;

#endif //OSPRATICE_TYPES_H
