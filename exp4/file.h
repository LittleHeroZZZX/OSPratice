//
// Created by littleherozzzx on 02/12/2022.
// 文件和文件目录管理
//
#include "stdio.h"
#include "types.h"
#include "string.h"
#include "block.h"
#include "time.h"


ssize_t create_dir(super_block* sb, fcb* dir, char* filename);

ssize_t create_file(super_block* sb, fcb* dir, char* filename, size_t size, void* content);

ssize_t do_create_file(super_block* sb, fcb* dir, char* filename, unsigned char attribute, size_t length);

ssize_t apply_inode(super_block* fcb);

int my_open(super_block* sb, char** args);

int do_open(super_block* sb, char* filePath);

int my_ls(super_block* sb, char** args);

void* do_read(super_block* sb, fcb* fcb, size_t size);

void do_write(super_block* sb, fcb* fcb, void* buff, size_t size);

size_t* get_blocks(super_block* sb, fcb* fcb);

void save_blocks(super_block* sb, fcb* fcb, size_t* blocks, size_t block_cnt);

void update_fcb(fcb* fcb, unsigned char attribute, size_t length, size_t file_cnt, unsigned char create);

int my_read(super_block* sb, user_open* _user_open, void* buf, size_t size);

int my_write(super_block* sb, char** args);

void* do_read_ch(void* stream);

void _do_write(super_block* sb, user_open* _user_open, void* buf, size_t size);

int my_cd(super_block* sb, char** args);

int my_mkdir(super_block* sb, char** args);

int my_pwd(super_block* sb, char** args);

int is_file_open(char* filePath,user_open** _user_open,int file_type);

fcb* index_to_fcb(super_block* sb, size_t index);

ssize_t dir_fcb_to_index(super_block* sb, fcb* fcb);

void do_printf(fcb* ptr,int format);

void do_copy(super_block* sb, char* src, char* dest);

void do_cat(super_block* sb, fcb* fcb);

ssize_t delete_file(super_block* sb, fcb* fcb, struct FCB* dir);

void clear_file(super_block* sb, fcb* fcb);


int my_rmdir(super_block* sb, char** args);
int my_create(super_block* sb, char** args);
int my_rm(super_block* sb, char** args);
int my_exit_sys(super_block* sb, char** args);
int my_close(super_block* sb, char** args);
int do_close(super_block* sb, char* filePath);
int my_touch(super_block* sb, char** args);
int my_clear(super_block* sb, char** args);
int my_cp(super_block* sb, char** args);
int my_cat(super_block* sb, char** args);

