/*
 * @Author: zhouxin zzzx@hdu.edu.cn
 * @Date: 2022-11-25 14:59:28
 * @LastEditTime: 2022-11-25 21:33:32
 * @Software: VSCode
 */




#ifndef MYFS_H
#define MYFS_H

#include "types.h"
#include "block.h"
#include "file.h"

void start_sys(char *bak_file, super_block **sb, int recreate);

void my_format(super_block **sb);

void recover(super_block **sb, char *bak_file);

void save(char *bak_file, super_block sb, size_t size);

void show_fs_info(super_block *sb);


#endif //MYFS_H
