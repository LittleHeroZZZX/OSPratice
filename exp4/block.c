//
// Created by littleherozzzx on 02/12/2022.
//

#include "block.h"


ssize_t allocate_block(super_block* sb, size_t block_count)
{
	size_t block = ERR_PARAM_INVALID;
	free_block_list* fb;

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
				list_add(&(((free_block_list*)((char*)fb + block_count * BLOCK_SIZE))->list), &fb->list);
				list_del(&fb->list);
			}
			break;
		}
	}
	return block;

}

void* index_to_addr(super_block* sb, size_t index)
{
	return sb->start_pos + index * BLOCK_SIZE;
}

__attribute__((unused)) size_t addr_to_index(super_block* sb, void* addr)
{
	return (addr - sb->start_pos) / BLOCK_SIZE;
}

void free_block(super_block* sb, size_t block_index, size_t block_count)
{
	free_block_list* block = (free_block_list*)index_to_addr(sb, block_index);
	free_block_list* fb = block;
	free_block_list* p;
	fb->count = block_count;
	fb->block_index = (size_t)((char*)block - (char*)sb->start_pos) / BLOCK_SIZE;
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

void merge_block(free_block_list* fbl1, free_block_list* fbl2)
{
	fbl1->count += fbl2->count;
	list_del(&fbl2->list);
}

void getFullPath(char* DestFullPath, char* filePath)
{
	char* currentDir = current_dir_name;
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	char fullPath[_MAX_PATH] = "\0";
	//文件路径解析
	_splitpath(filePath, NULL, dir, fname, ext);

	if (filePath[0] == '/')
	{
		strcpy(fullPath, filePath);
	}
	else if (!strncmp(filePath, "./", 2))
	{
		strcat(fullPath, currentDir);
		strcat(fullPath, filePath + 2);
	}
	else if (!strncmp(filePath, "..", 2))
	{
		char temp[_MAX_PATH];
		strcpy(temp, currentDir);
		char* ptr = strrchr(temp, '/');
		strncat(fullPath, currentDir, ptr - temp);
		strcat(fullPath, filePath + 2);
	}
	else
	{
		strcat(fullPath, currentDir);
		strcat(fullPath, filePath);
	}
	strcpy(DestFullPath, fullPath);
}

fcb* findFcb(super_block* sb, char* filePath)
{
	char fullPath[_MAX_PATH];
	getFullPath(fullPath, filePath);
	fcb* ptr = index_to_fcb(sb, sb->root_index);
	int flag = 0;
    char fileName[_MAX_FNAME];
	char* token = strtok(fullPath, "/");
	if(!strcmp(fullPath,"/")){
        flag=1;
    } else{
        while (token != NULL)
        {
            for (int i = 0; i < ptr->file_count; ++i)
            {
                inode* ptrInode = (inode*)do_read(sb, ptr, 0);
                strcpy(fileName,ptrInode[i].filename);
                if (!strcmp(ptrInode[i].filename, token))
                {
                    ptr = index_to_fcb(sb, ptrInode[i].inode_index);
                    break;
                }
            }
            char *nextToken = strtok(NULL, "/");
            if (nextToken == NULL&&!strcmp(fileName, token))
            {
                flag = 1;
            }
            if(nextToken!=NULL){
                strcpy(token,nextToken);
            } else{
                token=NULL;
            }
        }
    }
	if (flag == 0)
	{
		return NULL;
	}
	else
	{
		return ptr;
	}
}