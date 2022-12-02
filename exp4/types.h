//
// Created by littleherozzzx on 02/12/2022.
// 存放公共数据结构和宏定义常量
//

#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include <time.h>
#include "list.h"

extern char* current_dir;

#define BLOCK_SIZE 1024
#define SIZE 1024000
#define FILENAME_LEN 255
#define DIR_MAX_COUNT ((BLOCK_SIZE-sizeof(size_t))/sizeof(inode))

#define ORDINARY_FILE 0
#define DIRECTORY 1
#define ERR_NOT_ENOUGH_SPACE -1
#define ERR_PARAM_INVALID -2

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
    unsigned char attribute; // 文件属性 宏定义 ORDINARY_FILE 0 DIRECTORY 1
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
    inode root;
} super_block;

typedef struct user_open{
    fcb* f_fcb;
    size_t f_block_start;
    char path[256];
    size_t p_WR;
    char mode[8];
    unsigned char pcb_modified;
    unsigned char is_empty;
}user_open;

#endif //OSPRATICE_TYPES_H
