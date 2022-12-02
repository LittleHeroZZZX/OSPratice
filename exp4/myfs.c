/*
 * @Author: zhouxin zzzx@hdu.edu.cn
 * @Date: 2022-11-25 15:16:25
 * @LastEditTime: 2022-11-26 19:09:01
 * @Software: VSCode
 */

#include "myfs.h"
// 初始化文件系统
// 首先检查文件系统备份文件是否存在
// 从备份文件恢复或者格式化一个新的文件系统
// 所有的指针都要根据偏差重新修正

void recover(super_block **sb, char *bak_file)
{
    FILE *fp;
    void *fs;
    ptrdiff_t delta;
    fopen_s(&fp, bak_file, "rb");
    fs = malloc(SIZE);
    if (fs == NULL)
    {
        printf("malloc error\n");
        exit(1);
    }
    int cnt = fread(fs, 1, SIZE, fp);
    if (cnt != SIZE)
    {
        perror("fread error");
    }
    *sb = (super_block *)fs;

    delta = (char*)fs - (char*)(*sb)->start_pos;
    (*sb)->start_pos = fs;
    free_block_list *p_fbl = &(*sb)->free_block_list;
    update_list(p_fbl, list, delta);
    fclose(fp);
}

// 传递超级块指针的指针是为了让超级块指针指向0号内存
// 也就是将超级块置于文件系统的起始位置
void start_sys(char *bak_file, super_block **sb, int recreate)
{
    FILE *fp;
    fopen_s(&fp, bak_file, "rb");
    if (recreate)
    {
        printf("recreate file system\n");
        my_format(sb); 
        if (fp != NULL)
        {
            fclose(fp);
            remove(bak_file);
        }
    }
    else if (fp == NULL)
    {
        printf("No backup file found, creating a new file system.\n");
        my_format(sb);
    }
    else
    {
        printf("Backup file found, recovering file system.\n");
        fclose(fp);
        recover(sb, bak_file);
    }

}

// 格式化文件系统
// 包括初始化超级块、初始化空闲盘区链
void my_format(super_block ** p_sb)
{
    void *fs = malloc(SIZE);
    free_block_list *fb_node;  // fb_node用于指向第一个空闲盘块
    
    if (fs == NULL)
    {
        printf("malloc error\n");
        exit(1);
    }
    // 把超级块指针指向0号地址空间
    *p_sb = (super_block *)fs;
    
    super_block *sb = (super_block *)fs;
    sb->start_pos = fs;
    sb->block_count = (SIZE - sizeof(super_block)) / BLOCK_SIZE;
    sb->free_block_count = sb->block_count;
    sb->free_block_list_index = SIZE/BLOCK_SIZE - sb->block_count;
    INIT_LIST_HEAD(&sb->free_block_list);
    fb_node = (free_block_list *)(fs + sb->free_block_list_index*BLOCK_SIZE);
    fb_node->block_index = sb->free_block_list_index;
    fb_node->count = sb->block_count;
    list_add(&fb_node->list, &sb->free_block_list);






}


void save(char *bak_file, void* start_pos, size_t size)
{
    FILE *fp;
    size_t ret;
    fopen_s(&fp, bak_file, "wb+");
    if (fp == NULL)
    {
        printf("open file error\n");
        exit(1);
    }
    ret = fwrite(start_pos, 1, size, fp);
    if (ret != size) {
        printf("write error\n");
        exit(1);
    }
    fclose(fp);
}



void show(free_block_list *fbl)
{
    free_block_list *p;
    list_for_each_entry(p, &fbl->list, list)
    {
        printf("block_index: %d, count: %d\n", p->block_index, p->count);
    }
}


int main()
{
    super_block *sb;
    ptrdiff_t delta;
    start_sys("disc.bak",&sb, 1);
    show(&sb->free_block_list);
    save("disc.bak", sb->start_pos, SIZE);
    void* blocks[1000];
    for (int i=0; i<10; i++)
    {
        blocks[i] = allocate_block(sb, i+1);
        free_block_list *fb = (free_block_list*) blocks[i];
        delta = (char*)fb - (char*)sb;
        printf("i = %d\n", i);
        printf("block index: %lld\n", delta/BLOCK_SIZE);
        setbuf(stdout,NULL);
        printf("block_index: %d, count: %d, address: %p\n",
            fb->block_index, fb->count, fb);
        printf("first free block address: %p\n", sb->free_block_list.list.next);
        printf("\n");
    }
    show(&sb->free_block_list);
    for (int i=0; i<10; i+=2)
        free_block(sb, blocks[i], i+1);
    show(&sb->free_block_list);
    save("disc.bak", sb->start_pos, SIZE);
    return 0;
}