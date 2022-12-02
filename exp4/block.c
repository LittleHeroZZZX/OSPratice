//
// Created by littleherozzzx on 02/12/2022.
//

#include "block.h"

size_t allocate_block(super_block *sb, size_t block_count)
{
    // 分配盘块时，空出一个盘块，用于模拟真实情况
    size_t block = ERR_PARAM_INVALID;
    free_block_list *fb;

    if (block_count == 0)
    {
        return ERR_PARAM_INVALID;
    }
    if (sb->free_block_count < block_count + 1)
    {
        printf("No enough space\n");
        return ERR_NOT_ENOUGH_SPACE;
    }
    fb = &sb->free_block_list;
    list_for_each_entry(fb, &sb->free_block_list.list, list)
    {
        if (fb->count >= block_count)
        {
            block = fb->block_index;
            if (fb->count == block_count)
            {
                list_del(&fb->list);
            }
            else
            {
                fb->count -= block_count;
                fb->block_index += block_count;
                memcpy((char*)fb + block_count * BLOCK_SIZE, fb, sizeof(free_block_list));
                list_add(&(((free_block_list *) ((char*)fb + block_count * BLOCK_SIZE))->list), fb);
                list_del(&fb->list);
//                fb = list_entry(fb->list.next, free_block_list, list);
            }
            break;
        }
    }
    return block;

}

void* index_to_addr(super_block *sb, size_t index)
{
    return sb->start_pos + index * BLOCK_SIZE;
}

size_t addr_to_index(super_block *sb, void *addr)
{
    return (addr - sb->start_pos) / BLOCK_SIZE;
}

size_t free_block(super_block *sb, size_t block_index, size_t block_count)
{
    free_block_list * block = (free_block_list *) index_to_addr(sb, block_index);
    free_block_list *fb = block;
    free_block_list *p;
    fb->count = block_count;
    fb->block_index = (size_t)((char*)block - (char*)sb->start_pos)/BLOCK_SIZE;
    list_for_each_entry(p, &sb->free_block_list.list, list)
    {
        if (p->block_index > fb->block_index)
        {
            list_add(&fb->list, p->list.prev);
            break;
        }
    }
    list_for_each_entry(p, &sb->free_block_list.list, list)
    {
        if (p->block_index + p->count == list_entry(p->list.next, free_block_list, list)->block_index)
        {
            merge_block(p, list_entry(p->list.next, free_block_list, list));
        }
    }
}

void merge_block(free_block_list *fbl1, free_block_list *fbl2)
{
    fbl1->count += fbl2->count;
    list_del(&fbl2->list);
}