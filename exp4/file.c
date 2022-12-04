//
// Created by littleherozzzx on 02/12/2022.
//

#include "file.h"

/**
 * 创建文件 创建文件控制块、申请磁盘块、创建inode
 * @param sb
 * @param dir 文件所在目录的fcb 为NULL时创建根目录
 * @param filename 文件名
 * @param attribute 属性字段
 * @param length 文件大小（字节）
 */
int create_file(super_block *sb, fcb *dir,char *filename, unsigned char attribute, size_t length)
{
//    先申请磁盘块，然后申请inode，然后在inode对应的fcb中写入信息，然后在目录文件中写入该文件
    if (length <= 0) return ERR_PARAM_INVALID;
    size_t block_cnt = (length+BLOCK_SIZE-1)/BLOCK_SIZE;
    size_t block_index = allocate_block(sb, block_cnt);
    if (block_index > 0)
    {
        size_t inode_index = apply_inode(sb);
        if (inode_index == ERR_NOT_ENOUGH_INODE) return ERR_NOT_ENOUGH_INODE;
        fcb *fcb = &(sb->fcb_array[inode_index]);
        strcpy(fcb->filename, filename);
        update_fcb(fcb, attribute, length);
        if (dir != NULL) // 不是根目录
        {
            inode *inode = malloc(sizeof(inode));
            strcpy(inode->filename, filename);
            inode->length = length;
            inode->attribute = attribute;
            inode->inode_index = inode_index;
            do_write(sb, dir, inode, sizeof(inode));
            free(inode);
        }
    }
    else
    {
        return block_cnt;
    }
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
/**
 * 根据fcb获取存放文件的所有磁盘块号
 * @param sb 超级块
 * @param fcb 文件fcb
 * @return 存放文件的所有磁盘块号
 */
size_t *get_blocks(super_block *sb, fcb *fcb)
{
    size_t *blocks = (size_t *)malloc(sizeof(size_t) * (fcb->length + BLOCK_SIZE - 1) / BLOCK_SIZE);
    size_t block_cnt = (fcb->length + BLOCK_SIZE - 1) / BLOCK_SIZE;
    memcpy(blocks, fcb->mixed_index_block, sizeof(size_t) * block_cnt<LEVEL0_BLOCK_CNT?block_cnt:LEVEL0_BLOCK_CNT);
    if (block_cnt > LEVEL0_BLOCK_CNT)
    {
        memcpy(blocks + LEVEL0_BLOCK_CNT, index_to_addr(sb, fcb->mixed_index_block[LEVEL0_BLOCK_CNT]), block_cnt<LEVEL1_BLOCK_CNT+LEVEL0_BLOCK_CNT?block_cnt-LEVEL0_BLOCK_CNT:LEVEL1_BLOCK_CNT);
    }
    if (block_cnt > LEVEL1_BLOCK_CNT+LEVEL0_BLOCK_CNT)
    {
        block_cnt -= LEVEL1_BLOCK_CNT+LEVEL0_BLOCK_CNT;
        for (size_t i=0; i<block_cnt/(LEVEL1_BLOCK_CNT);i++)
            memcpy(blocks + LEVEL0_BLOCK_CNT + LEVEL1_BLOCK_CNT + i*LEVEL1_BLOCK_CNT, index_to_addr(sb,((size_t*) index_to_addr(sb, fcb->mixed_index_block[LEVEL0_BLOCK_CNT+1]))[i]), LEVEL1_BLOCK_CNT);
        if (block_cnt % LEVEL1_BLOCK_CNT != 0)
            memcpy(blocks + LEVEL0_BLOCK_CNT + LEVEL1_BLOCK_CNT + block_cnt/(LEVEL1_BLOCK_CNT)*LEVEL1_BLOCK_CNT, index_to_addr(sb,((size_t*) index_to_addr(sb, fcb->mixed_index_block[LEVEL0_BLOCK_CNT+1]))[block_cnt/(LEVEL1_BLOCK_CNT)]), block_cnt % LEVEL1_BLOCK_CNT);
    }
    return blocks;
}

/**
 * 从指定fcb中读取size个字节的数据
 * @param sb 超级块
 * @param fcb 文件控制块
 * @param size 0表示读取整个文件
 * @return
 */
void *do_read(super_block *sb,fcb *fcb, size_t size)
{
    size_t rest_size = size;
    size_t *blocks;
    void *buff;
    size = size == 0 ? fcb->length : size;
    buff = malloc(size);
    blocks = get_blocks(sb, fcb);
    size_t block_cnt = (fcb->length + BLOCK_SIZE - 1) / BLOCK_SIZE;
    for (size_t i = 0; i < block_cnt; i++)
    {
        memcpy(buff + i * BLOCK_SIZE, index_to_addr(sb, blocks[i]), rest_size > BLOCK_SIZE ? BLOCK_SIZE : rest_size);
        rest_size -= BLOCK_SIZE;
    }
    free(blocks);
    return buff;
}
/**
 * 将数据追加写入
 * @param sb 超级块
 * @param fcb 文件控制块
 * @param buff 数据
 * @param size 数据大小
 */
void do_write(super_block *sb, fcb *fcb, void *buff, size_t size)
{
    size_t *blocks, *new_blocks;
    size_t rest_size = size;
    size_t block_cnt = (fcb->length + BLOCK_SIZE - 1) / BLOCK_SIZE;
    blocks = get_blocks(sb, fcb);
    new_blocks = (size_t *)malloc((fcb->length + size + BLOCK_SIZE - 1) / BLOCK_SIZE);
    memcpy(new_blocks, blocks, sizeof(size_t) * block_cnt);
    free(blocks);
    memcpy(index_to_addr(sb, new_blocks[block_cnt-1])+fcb->length%BLOCK_SIZE, buff, BLOCK_SIZE - fcb->length%BLOCK_SIZE);
    rest_size -= BLOCK_SIZE - fcb->length%BLOCK_SIZE;
    for (size_t i = block_cnt; i < (fcb->length + size + BLOCK_SIZE - 1) / BLOCK_SIZE; i++)
    {
        new_blocks[i] = allocate_block(sb, 1);
        memcpy(index_to_addr(sb, new_blocks[i]), buff + size - rest_size, rest_size > BLOCK_SIZE ? BLOCK_SIZE : rest_size);
        rest_size -= BLOCK_SIZE;
    }
    save_blocks(sb, fcb, new_blocks, (fcb->length + size + BLOCK_SIZE - 1) / BLOCK_SIZE);
    //  先保存blocks，再更新fcb大小
    update_fcb(fcb, fcb->attribute, fcb->length + size);
}

/**
 * 在超级块的索引节点表中申请一个空闲的索引节点
 * @param fcb
 * @return 节点编号
 */
size_t apply_inode(super_block* fcb)
{
    for (size_t i=0; i<INODE_MAX_COUNT; i++)
        if (fcb->fcb_array[i].is_used == 0)
        {
            fcb->fcb_array[i].is_used = 1;
            return i;
        }
    return ERR_NOT_ENOUGH_INODE;
}
/**
 * 保存文件的混合索引块
 * @param sb 超级块
 * @param fcb 文件控制块
 * @param blocks 磁盘块号
 * @param block_cnt 磁盘块数量
 */
void save_blocks(super_block *sb, fcb *fcb, size_t *blocks, size_t block_cnt)
{
    size_t old_block_cnt = (fcb->length + BLOCK_SIZE - 1) / BLOCK_SIZE;
    size_t *old_blocks = get_blocks(sb, fcb);
    if (block_cnt <= old_block_cnt)
    {
    // 先释放多余磁盘块
        for (size_t i = block_cnt; i < old_block_cnt; i++)
        {
            free_block(sb, old_blocks[i], 1);
        }
    }
    memcpy(fcb->mixed_index_block, blocks, sizeof(size_t) * (block_cnt < LEVEL0_BLOCK_CNT ? block_cnt : LEVEL0_BLOCK_CNT));
    if (block_cnt > LEVEL0_BLOCK_CNT)
    {
        if(old_block_cnt <= LEVEL0_BLOCK_CNT)
        {
            fcb->mixed_index_block[LEVEL0_BLOCK_CNT] = allocate_block(sb, 1);
        }
        memcpy(index_to_addr(sb, fcb->mixed_index_block[LEVEL0_BLOCK_CNT]), blocks + LEVEL0_BLOCK_CNT, sizeof(size_t) * (block_cnt < LEVEL0_BLOCK_CNT + LEVEL1_BLOCK_CNT ? block_cnt - LEVEL0_BLOCK_CNT : LEVEL1_BLOCK_CNT));
    }
    if (block_cnt > LEVEL0_BLOCK_CNT + LEVEL1_BLOCK_CNT)
    {
        if(old_block_cnt <= LEVEL0_BLOCK_CNT + LEVEL1_BLOCK_CNT)
        {
            fcb->mixed_index_block[LEVEL0_BLOCK_CNT+1] = allocate_block(sb, 1);
        }
        size_t *level1_blocks = index_to_addr(sb, fcb->mixed_index_block[LEVEL0_BLOCK_CNT+LEVEL1_INDEX_CNT]);
        size_t level1_block_cnt = (block_cnt - LEVEL0_BLOCK_CNT - LEVEL1_BLOCK_CNT + LEVEL1_BLOCK_CNT - 1) / LEVEL1_BLOCK_CNT;
        size_t old_level1_block_cnt = (old_block_cnt - LEVEL0_BLOCK_CNT - LEVEL1_BLOCK_CNT + LEVEL1_BLOCK_CNT - 1) / LEVEL1_BLOCK_CNT;
        if (level1_block_cnt > old_level1_block_cnt)
            for (size_t i = old_level1_block_cnt; i < level1_block_cnt; i++)
                level1_blocks[i] = allocate_block(sb, 1);
        for (size_t i=0; i<level1_block_cnt; i++)
            memcpy(index_to_addr(sb, level1_blocks[i]), blocks + LEVEL0_BLOCK_CNT + LEVEL1_BLOCK_CNT + i * LEVEL1_BLOCK_CNT, sizeof(size_t) * (block_cnt < LEVEL0_BLOCK_CNT + LEVEL1_BLOCK_CNT + (i+1) * LEVEL1_BLOCK_CNT ? block_cnt - LEVEL0_BLOCK_CNT - LEVEL1_BLOCK_CNT - i * LEVEL1_BLOCK_CNT : LEVEL1_BLOCK_CNT));
    }


    free(old_blocks);

}

/**
 * @param fcb 文件控制块
 * 更新文件fcb，该操作会更新文件的修改时间
 * @param attribute 属性
 * @param length 文件大小
 */
void update_fcb(fcb *fcb, unsigned char attribute, size_t length)
{
    fcb->attribute = attribute;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    memcpy(&fcb->last_modify_time, t, sizeof(struct tm));
}