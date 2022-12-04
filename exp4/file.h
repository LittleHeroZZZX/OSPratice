//
// Created by littleherozzzx on 02/12/2022.
// 文件和文件目录管理
//
#include "stdio.h"
#include "types.h"
#include "string.h"
#include "block.h"
#include "time.h"




ssize_t create_file(super_block *sb, fcb *dir,char *filename, unsigned char attribute, size_t length);

ssize_t apply_inode(super_block* fcb);

__attribute__((unused)) user_open *my_open(char *fileName, __attribute__((unused)) int mode);

void *do_read(super_block *sb,fcb *fcb, size_t size);

void do_write(super_block *sb, fcb *fcb, void *buff, size_t size);

size_t *get_blocks(super_block *sb, fcb *fcb);

void save_blocks(super_block *sb, fcb *fcb, size_t *blocks, size_t block_cnt);

void update_fcb(fcb *fcb, unsigned char attribute, size_t length, size_t file_cnt, unsigned char create);

void my_fread(user_open *_user_open, char *buf,size_t size);

void my_fwrite(char *buf, size_t size, user_open *_user_open);

void my_cd(char *dir);

fcb* index_to_fcb(super_block *sb, size_t index);


