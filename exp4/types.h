#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include <time.h>
#include "stdlib.h"
#include "list.h"

#define CMD_TYPES 14


#define BLOCK_SIZE 1024
#define SIZE (512*1024*1024)
#define DISK_BACKUP_FILENAME "disk.bak"
#define FILENAME_LEN 255
#define DIR_MAX_COUNT ((BLOCK_SIZE-sizeof(size_t))/sizeof(inode))
#define FILE_MAX_COUNT 1024
#define INODE_MAX_COUNT FILE_MAX_COUNT

#define ORDINARY_FILE 0
#define DIRECTORY 1


// 索引块数量以及分布
#define LEVEL0_INDEX_CNT 10
#define LEVEL1_INDEX_CNT 1
#define LEVEL2_INDEX_CNT 1
#define INDEX_CNT (LEVEL0_INDEX_CNT+LEVEL1_INDEX_CNT+LEVEL2_INDEX_CNT)
#define LEVEL0_BLOCK_CNT LEVEL0_INDEX_CNT
#define LEVEL1_BLOCK_CNT (LEVEL1_INDEX_CNT*BLOCK_SIZE/sizeof(size_t))
#define LEVEL2_BLOCK_CNT (LEVEL2_INDEX_CNT*BLOCK_SIZE/sizeof(size_t)*BLOCK_SIZE/sizeof(size_t))

#define MAX_FILE_SIZE ((LEVEL0_BLOCK_CNT+LEVEL1_BLOCK_CNT+LEVEL2_BLOCK_CNT)*BLOCK_SIZE)

#define BASE_YEAR 1900
#define MAX_OPEN_FILE 10 //最多可同时打开文件个数
#define CSH_TOK_DELIM " "
#define MAX_ARG_LENGTH 64
#define MAX_CMD_LENGTH 256

#define ERR_NOT_ENOUGH_SPACE (-1)
#define ERR_PARAM_INVALID (-2)
#define ERR_NOT_ENOUGH_INODE (-3)
#define ERR_DUPLICATE_NAME (-4)
#define ERR_FILE_NOT_OPENED (-5)

#define FILE_ATTRIBUTES_TITLE 0
#define FILE_ATTRIBUTES 1

/**
 * 文件写模式
 * 由低位到高位分别为：读、写、写模式
 * 写模式：0表示覆盖写，1表示追加写
 */
#define OVERRIDE 0b001
#define APPEND 0b010
#define TRUNCATE 0b100


typedef struct free_block_list
{
	struct list_head list;
	// 空闲块数量
	size_t count;
	// 空闲块起始盘块
	size_t block_index;

} free_block_list;

typedef struct FCB
{
	char filename[256];
	// 0: directory, 1: file
	unsigned char attribute; // 文件属性 宏定义 ORDINARY_FILE 0 DIRECTORY 1
	unsigned char is_used; // 在超级块的索引节点表中是否被使用
	struct tm create_time;
	struct tm last_modify_time;
	size_t file_count; // 目录下文件数量

	size_t mixed_index_block[INDEX_CNT]; // 混合索引块，前10个直接索引块，第11个一级索引块，第12个二级索引块
	size_t length;
} fcb;

typedef struct inode
{
	char filename[255];
	size_t inode_index;
	// 0: directory, 1: file
	unsigned char attribute;
} inode;

typedef struct super_block
{
	size_t block_count;
	size_t free_block_count;
	size_t free_block_list_index;
	// 索引节点数组
	fcb* fcb_array;

	// 文件系统在内存中的起始地址
	void* start_pos;
	free_block_list free_block_list;
	size_t root_index;
} super_block;

typedef struct user_open
{
	fcb* f_fcb;            // FCB指针
	void* f_block_start;    // 文件的第一个物理块起始地址
	char path[256];    // 文件的完整绝对路径
	size_t p_WR; // 读写指针，指向硬盘中的地址
	int mode;    // 文件写入模式

} user_open;


extern fcb* current_dir;

extern char current_dir_name[_MAX_PATH];

extern char cmd[CMD_TYPES][6];

extern int (* cmd_func[])(super_block*, char**);

extern user_open* open_file_list[MAX_OPEN_FILE];

//extern int open_file_count;

extern int current_dir_fd;

#endif //OSPRATICE_TYPES_H
