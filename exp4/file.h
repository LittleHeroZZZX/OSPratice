//
// Created by littleherozzzx on 02/12/2022.
// 文件和文件目录管理
//
#include "stdio.h"
#include "types.h"
#include "string.h"
#include "block.h"



void create_fcb(fcb *fcb, char *filename, unsigned char attribute, size_t length);

int create_file(super_block *sb, fcb dir,char *filename, unsigned char attribute, size_t length);

void apply_inode(super_block *sb, inode *inode, char *filename, unsigned char attribute);

user_open *my_open(char *fileName, int mode);

void *do_read(super_block *sb,fcb *fcb, size_t size)

size_t *get_blocks(super_block *sb, fcb *fcb);

void my_fread(user_open *_user_open, char *buf,size_t size);

void my_fwrite(char *buf, size_t size, user_open *_user_open);

void my_cd(char *dir);


