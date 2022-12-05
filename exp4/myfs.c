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

void recover(super_block **sb, char *bak_file) {
    FILE *fp;
    void *fs;
    ptrdiff_t delta;
    fopen_s(&fp, bak_file, "rb");
    fs = malloc(SIZE);
    if (fs == NULL) {
        printf("malloc error\n");
        exit(1);
    }
    size_t cnt = fread(fs, 1, SIZE, fp);
    if (cnt != SIZE) {
        perror("fread error");
    }
    *sb = (super_block *) fs;

    delta = (char *) fs - (char *) (*sb)->start_pos;
    (*sb)->start_pos = fs;
    free_block_list *p_fbl = &(*sb)->free_block_list;
    update_list(p_fbl, list, delta)
    (*sb)->fcb_array = (void *) (*sb)->fcb_array + delta;
    fclose(fp);
}

// 传递超级块指针的指针是为了让超级块指针指向0号内存
// 也就是将超级块置于文件系统的起始位置
void start_sys(char *bak_file, super_block **sb, int recreate) {
    FILE *fp;
    fopen_s(&fp, bak_file, "rb");
    if (recreate) {
        printf("recreate file system\n");
        my_format(sb);
        if (fp != NULL) {
            fclose(fp);
            remove(bak_file);
        }
    } else if (fp == NULL) {
        printf("No backup file found, creating a new file system.\n");
        my_format(sb);
    } else {
        printf("Backup file found, recovering file system.\n");
        fclose(fp);
        recover(sb, bak_file);
    }

}

// 格式化文件系统
// 包括初始化超级块、初始化空闲盘区链
void my_format(super_block **p_sb) {
    void *fs = malloc(SIZE);
    free_block_list *fb_node;  // fb_node用于指向第一个空闲盘块

    if (fs == NULL) {
        printf("malloc error\n");
        exit(1);
    }
    // 把超级块指针指向0号地址空间
    *p_sb = (super_block *) fs;

    super_block *sb = (super_block *) fs;
    sb->start_pos = fs;
    sb->block_count = SIZE / BLOCK_SIZE;
    sb->free_block_count = (SIZE - sizeof(super_block)) / BLOCK_SIZE;
    sb->free_block_list_index = SIZE / BLOCK_SIZE - sb->free_block_count;
    INIT_LIST_HEAD(&sb->free_block_list.list);
    fb_node = (free_block_list *) (fs + sb->free_block_list_index * BLOCK_SIZE);
    fb_node->block_index = sb->free_block_list_index;
    fb_node->count = sb->free_block_count;
    list_add(&fb_node->list, &sb->free_block_list.list);

    size_t fcb_array_block_index = allocate_block(sb, (sizeof(fcb) * INODE_MAX_COUNT + BLOCK_SIZE - 1) / BLOCK_SIZE);
    if (fcb_array_block_index != 1) {
        printf("fcb_array_block_index error\n");
        exit(1);
    }
    sb->fcb_array = index_to_addr(sb, fcb_array_block_index);
    ssize_t fcb_index = do_create_file(sb, NULL, "/", DIRECTORY, 0);
    if (fcb_index < 0) {
        printf("create root directory error\n");
        exit(1);
    }
    sb->root_index = fcb_index;
    size_t users_inode = create_dir(sb, index_to_fcb(sb, fcb_index), "users");
    create_dir(sb, index_to_fcb(sb, users_inode), "root");
    create_dir(sb, index_to_fcb(sb, users_inode), "guest");
    create_dir(sb, index_to_fcb(sb, sb->root_index), "groups");


}


void save(char *bak_file, super_block sb, size_t size) {
    FILE *fp;
    size_t ret;
    void *start_pos = sb.start_pos;
    fopen_s(&fp, bak_file, "wb+");
    if (fp == NULL) {
        printf("open file error\n");
        exit(1);
    }
    ret = fwrite(start_pos, 1, size, fp);
    if (ret != size) {
        printf("write error\n");
        exit(1);
    }
//  保存索引节点数组
    ret = fwrite(sb.fcb_array, 1, sizeof(fcb) * INODE_MAX_COUNT, fp);
    if (ret != sizeof(fcb) * INODE_MAX_COUNT) {
        printf("write error\n");
        exit(1);
    }
    fclose(fp);
}


__attribute__((unused)) void show(free_block_list *fbl) {
    free_block_list *p;
    list_for_each_entry(p, &fbl->list, list) {
        printf("block_index: %lld, count: %lld\n", p->block_index, p->count);
    }
}

void show_dirs(super_block *sb, fcb *fcb, size_t level) {
    inode *files_inodes = (inode *) do_read(sb, fcb, 0);
    for (int i = 0; i < fcb->file_count; i++) {
        if (files_inodes[i].attribute == DIRECTORY) {
            for (int j = 0; j < level; j++) {
                printf("    ");
            }
            printf("%s(dir)\n", files_inodes[i].filename);
            if (strcmp(files_inodes[i].filename, ".") != 0 && strcmp(files_inodes[i].filename, "..") != 0)
                show_dirs(sb, index_to_fcb(sb, files_inodes[i].inode_index), level + 1);
        } else {
            for (int j = 0; j < level; j++) {
                printf("    ");
            }
            printf("%s(file)\n", files_inodes[i].filename);
        }
    }
    free(files_inodes);

}

void show_fs_info(super_block *sb) {
    printf("******************************************\n");
    printf("******************************************\n");
    printf("%-20s: %lldMB %lldKB\n", "disk space size",
           sb->block_count * BLOCK_SIZE / 1024 / 1024 , sb->block_count * BLOCK_SIZE / 1024 % 1024);
    printf("%-20s: %lldMB %lldKB\n", "free space size",
           sb->free_block_count * BLOCK_SIZE / 1024 / 1024 , sb->free_block_count * BLOCK_SIZE / 1024 % 1024);
    printf("%-20s: %lldKB\n", "block size", BLOCK_SIZE / 1024);
    printf("%-20s: %lldMB %lldKB \n", "max file size",
           MAX_FILE_SIZE / 1024 / 1024 , MAX_FILE_SIZE / 1024 % 1024);

    printf("******************************************\n");
    printf("******************************************\n");


}

int main() {

    super_block *sb;
    start_sys("disk", &sb, 1);
    show_fs_info(sb);
    save("disc.bak", *sb, SIZE);
    create_dir(sb, index_to_fcb(sb, sb->root_index), "test1");
    printf("/(dir)\n");
    show_dirs(sb, index_to_fcb(sb, sb->root_index), 1);
    my_cd(sb, "/users/");
    printf("%s\n", current_dir_name);
    printf("%s\n", current_dir->filename);
    my_cd(sb, "./guest");
    printf("%s\n", current_dir_name);
    printf("%s\n", current_dir->filename);
    my_cd(sb, "../groups");
    printf("%s\n", current_dir_name);
    printf("%s\n", current_dir->filename);
    show_fs_info(sb);
    return 0;
}

