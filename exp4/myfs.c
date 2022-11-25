/*
 * @Author: zhouxin zzzx@hdu.edu.cn
 * @Date: 2022-11-25 15:16:25
 * @LastEditTime: 2022-11-25 15:55:24
 * @Software: VSCode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myfs.h"

// 初始化文件系统
// 首先检查文件系统备份文件是否存在
// 从备份文件恢复或者格式化一个新的文件系统
void start_sys(char *bak_file, super_block *sb)
{
    FILE *fp;
    fp = fopen(bak_file, "r");
    if (fp == NULL)
    {
        printf("No backup file found, creating a new file system.\n");
        my_format(sb);
    }
    else
    {
        printf("Backup file found, recovering file system.\n");
        my_revover();
    }

}

// 格式化文件系统
// 包括初始化超级块、初始化空闲盘区链
void my_format(super_block *sb)
{
    void *fs = malloc(SIZE);
    if (fs == NULL)
    {
        printf("malloc error\n");
        exit(1);
    }
    sb->block_count = (SIZE - sizeof(super_block)) / BLOCK_SIZE;
    sb->free_block_count = sb->block_count;
    sb->free_block_list_index = SIZE/BLOCK_SIZE - sb->block_count;
    sb->free_block_list = (free_block_list *)(fs + sb->free_block_list_index*BLOCK_SIZE);
    INIT_LIST_HEAD(&sb->free_block_list->list);
    sb->free_block_list->count = sb->free_block_count;
    sb->free_block_list->block_index = 0;
}

void my_revover()
{
    printf("1");
}

int main()
{
    super_block sb;
    start_sys("bak_file", &sb);
    return 0;
}