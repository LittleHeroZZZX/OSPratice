//
// Created by littleherozzzx on 02/12/2022.
// 文件和文件目录管理
//
#include "stdio.h"
#include "types.h"
#include "string.h"

#ifndef OSPRATICE_FILE_H
#define OSPRATICE_FILE_H

void create_fcb(fcb *fcb, char *filename, unsigned char attribute, size_t length);

void my_fread(user_open *_user_open, char *buf,size_t size);

void my_fwrite(char *buf, size_t size, user_open *_user_open);

void my_cd(char *dir);

#endif //OSPRATICE_FILE_H
