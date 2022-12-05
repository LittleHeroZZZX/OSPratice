//
// Created by littleherozzzx on 02/12/2022.
// 文件和文件目录管理
//
#include "stdio.h"
#include "types.h"
#include "string.h"
#include "block.h"
#include "time.h"


ssize_t create_dir(super_block *sb, fcb *dir, char *filename);

ssize_t create_file(super_block *sb, fcb *dir, char *filename, size_t size, void *content);

ssize_t do_create_file(super_block *sb, fcb *dir, char *filename, unsigned char attribute, size_t length);

ssize_t apply_inode(super_block *fcb);

user_open *my_open(super_block *sb, char *filePath, int mode);

void *my_ls(super_block *sb, char *filePath);

void *do_read(super_block *sb, fcb *fcb, size_t size);

void do_write(super_block *sb, fcb *fcb, void *buff, size_t size);

size_t *get_blocks(super_block *sb, fcb *fcb);

void save_blocks(super_block *sb, fcb *fcb, size_t *blocks, size_t block_cnt);

void update_fcb(fcb *fcb, unsigned char attribute, size_t length, size_t file_cnt, unsigned char create);

void f_read(super_block *sb, user_open *_user_open, void *buf, size_t size);

void f_write(char *buf, size_t size, user_open *_user_open);

void *my_cd(super_block *sb, char *filePath);

fcb *index_to_fcb(super_block *sb, size_t index);

void my_cat(super_block *sb, fcb *fcb);



