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

/**
 * 物理磁盘块号转物理地址
 * @param sb
 * @param index 物理磁盘块号
 * @return 物理地址
 */

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

static char* left(char *dest,const char *src ,int n){
    char *p=dest;
    char *q=src;
    int len=strlen(src);
    if(n>len){
        n=len;
    }
    while(n--) *(p++)=*(q++);
    *(p++)='\0';
    return dest;
}
/**
 * 将目的路径和当前路径连接起来，然后进行解析，结果保存在DestFullPath中，但并不能保证目录中就有这个文件，需要配合findFcb使用
 * @param DestFullPath
 * @param filePath
 */
void getFullPath(char* DestFullPath, char* filePath)
{
   if(filePath[0]=='/'){
	   int i=0;
	   //遇到连续多个"///////"，忽略
	   for (i = 0; filePath[i]=='/'; ++i);
       strcpy(DestFullPath,filePath+i-1);
	   if (strcmp(DestFullPath,"/")&&DestFullPath[strlen(DestFullPath)-1]!='/'){
		   strcat(DestFullPath,"/");
	   }
   } else{
       char str[_MAX_PATH];
       strcpy(str,current_dir_name);
       strcat(str,filePath);
       char fullPath[_MAX_PATH] ="/";
       char *token;
       token = strtok(str,"/");
       while (token!=NULL){
           char fileName[_MAX_FNAME];
           strcpy(fileName,token);
           if(!strcmp(fileName,"..")){
               char parentPath[_MAX_PATH];
               left(parentPath,fullPath,strlen(fullPath)-1);
               char* ptr = strrchr(fullPath, '/');
               char* ptr2 = strrchr(parentPath, '/');
               int offset;
               if(ptr2==NULL){
                   offset = (int) (ptr - fullPath);
                   strcpy(fullPath,fullPath+offset);
               } else{
                   left(fullPath,fullPath,strlen(fullPath) - strlen(ptr2));
               }
           }else if(!strcmp(fileName,".")){
           } else{
               strcat(fileName,"/");
               strcat(fullPath,fileName);
           }
           token = strtok(NULL,"/");
       }
       strcpy(DestFullPath,fullPath);
   }
}


/**
 * 通过文件路径找到文件的fcb*
 * @param filePath
 * @return fcb* 找不到则返回NULL
 */
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
        while (token != NULL){
            for (int i = 0; i < ptr->file_count; ++i){
                inode* ptrInode = (inode*)do_read(sb, ptr, 0);
                strcpy(fileName,ptrInode[i].filename);
                if (!strcmp(ptrInode[i].filename, token)){
                    ptr = index_to_fcb(sb, ptrInode[i].inode_index);
                    break;
                }
            }
            char *nextToken = strtok(NULL, "/");
            if (nextToken == NULL&&!strcmp(fileName, token)){
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