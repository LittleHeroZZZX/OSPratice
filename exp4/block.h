//
// Created by littleherozzzx on 02/12/2022.
// 磁盘块管理（分配、回收）
//

#ifndef BLOCK_H
#define BLOCK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "myfs.h"

ssize_t allocate_block(super_block *sb, size_t block_count);
void free_block(super_block *sb, size_t block_index, size_t block_count);
void merge_block(free_block_list *fbl1, free_block_list *fbl2);
void* index_to_addr(super_block *sb, size_t index);

/**
 * 根据文件路径返回对应FCB
 * @param sb
 * @param filePath
 * @return
 */
fcb* findFcb(super_block *sb,char *filePath);

/**
 * 根据文件路径获得其全路径
 * @param DestFullPath 全路径
 * @param filePath 文件路径
 */
void getFullPath(char *DestFullPath,char* filePath);

__attribute__((unused)) size_t addr_to_index(super_block *sb, void *addr);

#endif //BLOCK_H
