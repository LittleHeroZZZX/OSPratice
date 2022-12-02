//
// Created by littleherozzzx on 02/12/2022.
//

#ifndef BLOCK_H
#define BLOCK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "myfs.h"

void* allocate_block(super_block *sb, size_t block_count);
void free_block(super_block *sb, void *block, size_t block_count);
void merge_block(free_block_list *fbl1, free_block_list *fbl2);

#endif //BLOCK_H
