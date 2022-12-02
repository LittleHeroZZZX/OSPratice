//
// Created by littleherozzzx on 02/12/2022.
//

#include "file.h"
void create_fcb(fcb *fcb, char *filename, unsigned char attribute, size_t length)
{
    strcpy(fcb->filename, filename);
    fcb->attribute = attribute;
    fcb->length = length;
    time_t t = time(NULL);
    fcb->create_time = *localtime(&t);
    fcb->last_modify_time = *localtime(&t);
}
/**
 * 打开文件
 * @param fileName 文件名及路径
 * @param mode 打开模式（r/w/w+/rw/o/a）
 * @return
 */
user_open *my_open(char *fileName, int mode){
    extern char *current_dir;
    const char ch = '/';
    char *realFileName = strrchr(fileName, ch);
    if(realFileName == NULL){

    }else{

    }
}

void my_fread(user_open *_user_open, char *buf,size_t size)
{
    // ToDo
}

void my_fwrite(char *buf, size_t size, user_open *_user_open)
{
    // ToDo
}


void my_cd(char* dir)
{
    // ToDo
}