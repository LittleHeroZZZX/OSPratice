//
// Created by littleherozzzx on 02/12/2022.
//

#include "file.h"

extern fcb* current_dir;

/**
 * 创建文件 创建文件控制块、申请磁盘块、创建inode
 * @param sb
 * @param dir 文件所在目录的fcb 为NULL时创建根目录
 * @param filename 文件名
 * @param attribute 属性字段
 * @param length 文件大小（字节）
 * @return inode序号
 */
ssize_t do_create_file(super_block* sb, fcb* dir, char* filename, unsigned char attribute, size_t length)
{
//    先申请磁盘块，然后申请inode，然后在inode对应的fcb中写入信息，然后在目录文件中写入该文件
	size_t block_cnt = (length + BLOCK_SIZE - 1) / BLOCK_SIZE;
	if (block_cnt == 0) block_cnt = 1;
	ssize_t block_index = allocate_block(sb, block_cnt);
	if (block_index >= 0)
	{
		ssize_t inode_index = apply_inode(sb);
		if (inode_index == ERR_NOT_ENOUGH_INODE) return ERR_NOT_ENOUGH_INODE;
		fcb* fcb = &(sb->fcb_array[inode_index]);
		strcpy(fcb->filename, filename);
		update_fcb(fcb, attribute, length, 0, 1);
		if (dir != NULL) // 不是根目录
		{
			inode* new_inode = malloc(sizeof(inode));
			strcpy(new_inode->filename, filename);
			new_inode->attribute = attribute;
			new_inode->inode_index = inode_index;
			do_write(sb, dir, new_inode, sizeof(inode));
//            do write中已经更新了length
			update_fcb(dir, dir->attribute, dir->length, dir->file_count + 1, 0);

		}
		return inode_index;
	}
	else
	{
		return block_index;
	}
}
/**
 * 从open_file_list获取一个未被使用的user_open
 * @return 找到返回对应的下标，满了返回-1
 */
int get_user_open() {
    int i;
    for (i = 0; i < MAX_OPEN_FILE; i++) {
        if (open_file_list[i].is_empty) {
            return i;
        }
    }
    return -1;
}
/**
 * 根据文件路径和打开模式打开文件，加入open_file_list中
 * @param sb
 * @param filePath
 * @param mode 文件打开模式，传入NULL默认为“r”
 * @return open_file_list 的下标
 */
int do_open(super_block* sb, char* filePath,char *mode){
    int fd = get_user_open();
    if (fd==-1){
        fprintf(stderr, "\"open error\": The number of file openings is maximum\n");
        return -1;
    }
    open_file_list[fd].f_fcb = findFcb(sb,filePath);
    open_file_list[fd].is_empty = 0;
    if (strcmp(mode,"r")){
        open_file_list[fd].mode = READ;
    } else if(!strcmp(mode,"w")){
        open_file_list[fd].mode = WRITE;
    } else if(strcmp(mode,"a")){
        open_file_list[fd].mode = WRITE;
    } else{
        fprintf(stderr, "\"open error\": The %s mode could not be found\n", mode);
        return -1;
    }
    return fd;
}

/**
 * 打开单个文件或者目录文件 ，-l查看所有打开的文件
 * @param sb
 * @param args
 * @return
 */
int my_open(super_block* sb, char **args)
{
    int fd;
    //参数为空
    if (args[1] == NULL) {
        fprintf(stderr, "open: missing argument!\n");
        return 1;
    }

    // -l查看所有已经打开的文件
    if (args[1][0] == '-') {
        if (!strcmp(args[1], "-l")) {
            printf("filename\tlength\tattribute\tcreate time\tlast modify time\t\n");
            for (int i = 0; i < MAX_OPEN_FILE; i++) {
                if(!open_file_list[i].is_empty){
                    printf("%s\t\t%d\t%s\t%d-%d-%d %d:%d\t%d-%d-%d %d:%d\t\n", open_file_list[i].f_fcb->filename, open_file_list[i].f_fcb->length,
                           open_file_list[i].f_fcb->attribute == 1 ? "directory" : "file", open_file_list[i].f_fcb->create_time.tm_year + BASE_YEAR,
                           open_file_list[i].f_fcb->create_time.tm_mon, open_file_list[i].f_fcb->create_time.tm_mday, open_file_list[i].f_fcb->create_time.tm_hour, open_file_list[i].f_fcb->create_time.tm_min,
                           open_file_list[i].f_fcb->last_modify_time.tm_year + BASE_YEAR, open_file_list[i].f_fcb->last_modify_time.tm_mon, open_file_list[i].f_fcb->last_modify_time.tm_mday,
                           open_file_list[i].f_fcb->last_modify_time.tm_hour, open_file_list[i].f_fcb->last_modify_time.tm_min);
                }
            }
            return 1;
        } else {
            fprintf(stderr, "\"open error\": wrong argument\n");
            return 1;
        }
    }

    char *filePath =NULL;
    char *mode = NULL;
    if(args[1]!=NULL){
        filePath = args[1];
    }else{
        fprintf(stderr, "\"open error\": missing path argument\n", args[1]);
        return 1;
    }
    if(args[1]!=NULL){
        mode = args[2];
    }else{
        fprintf(stderr, "\"open error\": missing mode argument\n", args[1]);
        return 1;
    }

    //如果当前文件已经被打开
    fcb *fcb = findFcb(sb,filePath);
    for (int i = 0; i < MAX_OPEN_FILE; i++) {
        if (open_file_list[i].is_empty == 0) {
            if (fcb == open_file_list[i].f_fcb) {
                fprintf(stderr, "\"open error\": cannot open %s: File or folder is open\n", args[i]);
                return 1;
            }
        }
    }

    //可以打开一个目录文件，但并不会更改当前工作目录。
    do_open(sb,filePath,mode);

	return 1;
}

/**
 * 打印对应目录下的的所有文件
 * @param sb
 * @param filePath
 * @return
 */
int my_ls(super_block* sb, char** args)
{
    char * filePath =NULL;
    if(args[1]!=NULL){
        filePath = args[1];
    }
	if (filePath == NULL)
	{
		filePath = (char*)malloc(sizeof(char) * _MAX_PATH);
		strcpy(filePath, current_dir_name);
	}
	fcb* dirFcb = findFcb(sb, filePath);
	if (dirFcb == NULL || dirFcb->attribute == ORDINARY_FILE)
	{
        fprintf(stderr, "\"ls\": cannot open %s: No such file or folder\n", filePath);
        return 1;
	}
	printf("filename\tlength\tattribute\tcreate time\tlast modify time\t\n");
	inode* ptrInode = (inode*)do_read(sb, dirFcb, 0);
	for (int i = 0; i < dirFcb->file_count; ++i)
	{
		fcb* ptr = index_to_fcb(sb, ptrInode[i].inode_index);
		printf("%s\t\t%d\t%s\t%d-%d-%d %d:%d\t%d-%d-%d %d:%d\t\n", ptr->filename, ptr->length,
			ptr->attribute == 1 ? "directory" : "file", ptr[i].create_time.tm_year + BASE_YEAR,
			ptr->create_time.tm_mon, ptr->create_time.tm_mday, ptr->create_time.tm_hour, ptr->create_time.tm_min,
			ptr[i].last_modify_time.tm_year + BASE_YEAR, ptr->last_modify_time.tm_mon, ptr->last_modify_time.tm_mday,
			ptr->last_modify_time.tm_hour, ptr->last_modify_time.tm_min);
	}
	printf("\n");
    return 1;
}

/**
 * cd 更改当前工作目录
 * @param sb
 * @param args
 * @return
 */
int my_cd(super_block* sb, char** args)
{
    int fd;
    char * filePath =NULL;
    if(args[1]!=NULL){
        filePath = args[1];
    }
    if (filePath == NULL){
        filePath = (char*)malloc(sizeof(char) * _MAX_PATH);
        strcpy(filePath, current_dir_name);
    }

	fcb* fcb = findFcb(sb, filePath);

	if (fcb == NULL) {
        fprintf(stderr, "\"cd\" error: cannot open %s: No such folder\n", filePath);
        return 1;
    }
    if(fcb->attribute==ORDINARY_FILE){
        fprintf(stderr, "\"cd\" error: cannot open %s: It is a file!\n", filePath);
        return 1;
    }

    //关闭当前目录文件
    do_close(sb,current_dir_name);

    // 文件未打开，需要先打开这个文件然后再cd过去
    if ((fd = do_open(sb,filePath,"rw")) > 0) {
        current_dir_fd = fd;
        current_dir = fcb;
        getFullPath(current_dir_name, filePath);
    }
    return 1;
}

void my_mkdir(super_block* sb, char* dirname)
{
	if (dirname == NULL)
	{
		printf("Dir name is not set.");
		return;
	}

	fcb* dirFcb = current_dir;
	inode* p_inode = (inode*)do_read(sb, dirFcb, 0);
	for (int i = 0; i < dirFcb->file_count; ++i)
	{
		fcb* _fcb = index_to_fcb(sb, p_inode[i].inode_index);
		if (!strcmp(_fcb->filename, dirname))
		{
			printf("There is duplicate name file in current dir.");
			return;
		}
	}
	create_dir(sb, current_dir, dirname);
	return;
}

int my_pwd()
{
	printf("%s\n",current_dir_name);
    return 1;
}

/**
 * 根据fcb获取存放文件的所有磁盘块号
 * @param sb 超级块
 * @param fcb 文件fcb
 * @return 存放文件的所有磁盘块号
 */
size_t* get_blocks(super_block* sb, fcb* fcb)
{
	if (fcb->length == 0) return NULL;
	size_t* blocks = (size_t*)malloc(sizeof(size_t) * ((fcb->length + BLOCK_SIZE - 1) / BLOCK_SIZE));
	size_t block_cnt = (fcb->length + BLOCK_SIZE - 1) / BLOCK_SIZE;
	memcpy(blocks, fcb->mixed_index_block,
		sizeof(size_t) * (block_cnt < LEVEL0_BLOCK_CNT ? block_cnt : LEVEL0_BLOCK_CNT));
	if (block_cnt > LEVEL0_BLOCK_CNT)
	{
		memcpy(blocks + LEVEL0_BLOCK_CNT, index_to_addr(sb, fcb->mixed_index_block[LEVEL0_BLOCK_CNT]),
			sizeof(size_t)
				* (block_cnt < LEVEL1_BLOCK_CNT + LEVEL0_BLOCK_CNT ? block_cnt - LEVEL0_BLOCK_CNT : LEVEL1_BLOCK_CNT));
	}
	if (block_cnt > LEVEL1_BLOCK_CNT + LEVEL0_BLOCK_CNT)
	{
		block_cnt -= LEVEL1_BLOCK_CNT + LEVEL0_BLOCK_CNT;
		for (size_t i = 0; i < block_cnt / (LEVEL1_BLOCK_CNT); i++)
			memcpy(blocks + LEVEL0_BLOCK_CNT + LEVEL1_BLOCK_CNT + i * LEVEL1_BLOCK_CNT,
				index_to_addr(sb, ((size_t*)index_to_addr(sb, fcb->mixed_index_block[LEVEL0_BLOCK_CNT + 1]))[i]),
				LEVEL1_BLOCK_CNT * sizeof(size_t));
		if (block_cnt % LEVEL1_BLOCK_CNT != 0)
			memcpy(blocks + LEVEL0_BLOCK_CNT + LEVEL1_BLOCK_CNT + block_cnt / (LEVEL1_BLOCK_CNT) * LEVEL1_BLOCK_CNT,
				index_to_addr(sb, ((size_t*)index_to_addr(sb, fcb->mixed_index_block[LEVEL0_BLOCK_CNT + 1]))[
					block_cnt / (LEVEL1_BLOCK_CNT)]), block_cnt % LEVEL1_BLOCK_CNT * sizeof(size_t));
	}
	return blocks;
}

/**
 * 读取文件
 * @param sb 超级块
 * @param _user_open 待读取文件的user_open
 * @param buf 读取到的文件内容
 * @param size 读取的长度
 * @return
 */
void my_read(super_block* sb, user_open* _user_open, void* buf, size_t size)
{
	if (_user_open->f_fcb->attribute == DIRECTORY)
	{
		printf("File to read is not a file!\n");
		return;
	}
	if (!(_user_open->mode & READ))
	{
		printf("File to read is not readable!\n");
		return;
	}
	if (!buf)
	{
		printf("Buffer is null!\n");
		return;
	}

	fcb* fcb = _user_open->f_fcb;
	size_t rest_size = fcb->length > size ? size : fcb->length;
	size_t offset = _user_open->p_WR;
	size_t* blocks = get_blocks(sb, fcb);
	size_t block_cnt = (fcb->length + BLOCK_SIZE - 1) / BLOCK_SIZE;

	_user_open->p_WR = blocks[0];
	for (size_t i = 0; i < block_cnt; i++)
	{
		memcpy(buf + i * BLOCK_SIZE, index_to_addr(sb, blocks[i]), rest_size > BLOCK_SIZE ? BLOCK_SIZE : rest_size);
		rest_size -= BLOCK_SIZE;
		_user_open->p_WR += BLOCK_SIZE;
	}
	free(blocks);
	_user_open->p_WR = offset;
	return;
}

/**
 * 写入文件
 * @param sb 超级块
 * @param _user_open 待写入文件的user_open
 * @param buf 待写入的文件内容
 * @param size 写入的长度
 * @return
 */
void my_write(super_block* sb, user_open* _user_open, void* buf, size_t size)
{
	if (!sb | !_user_open || !buf)return;
	if (_user_open->f_fcb->attribute == DIRECTORY)
	{
		printf("File to write is not a file!\n");
		return;
	}
	if (!(_user_open->mode & WRITE))
	{
		printf("File to write is not writable!\n");
		return;
	}

	fcb* _fcb = _user_open->f_fcb;
	size_t* old_blocks, * new_blocks;
	size_t old_block_cnt = (_fcb->length + BLOCK_SIZE - 1) / BLOCK_SIZE;
	size_t old_block_frag = _fcb->length % BLOCK_SIZE; //原始文件最后一块的块大小
	size_t new_block_size = 0;
	size_t new_block_cnt = 0;
	size_t rest_size = size;
	size_t size_to_write = 0;
	int mode = _user_open->mode;

	switch (mode & APPEND)
	{
	case 0b100:
		// append mode
		_user_open->p_WR = _fcb->length;
		old_blocks = get_blocks(sb, _fcb);
		new_block_size = _fcb->length + size;
		new_block_cnt = (new_block_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
		if (old_block_cnt)
		{
			size_to_write = BLOCK_SIZE - old_block_frag;
			memcpy(index_to_addr(sb, old_blocks[old_block_cnt - 1]) + old_block_frag, buf, size_to_write);
			rest_size -= size_to_write;
			_user_open->p_WR += size_to_write;
		}
		new_blocks = (size_t*)malloc(sizeof(size_t) * new_block_cnt);
		for (size_t i = 0; i < old_block_cnt; i++)
		{
			new_blocks[i] = allocate_block(sb, 1);
			size_to_write = rest_size > BLOCK_SIZE ? BLOCK_SIZE : rest_size;
			memcpy(index_to_addr(sb, new_blocks[i]), buf + size - rest_size, size_to_write);
			rest_size -= size_to_write;    // do_write中减去了BLOCK_SIZE
			_user_open->p_WR += size_to_write;
		}
		break;
	case 0b000:
		// override mode
		_user_open->p_WR = 0;
		free_block(sb, _user_open->f_block_start, old_block_cnt);
		new_block_size = size;
		new_block_cnt = (new_block_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
		new_blocks = (size_t*)malloc(sizeof(size_t) * new_block_cnt);
		for (size_t i = 0; i < new_block_cnt; i++)
		{
			new_blocks[i] = allocate_block(sb, 1);
			size_to_write = rest_size > BLOCK_SIZE ? BLOCK_SIZE : rest_size;
			memcpy(index_to_addr(sb, new_blocks[i]), buf + size - rest_size, size_to_write);
			rest_size -= size_to_write;
			_user_open->p_WR += size_to_write;
		}
		break;
	}
	save_blocks(sb, _fcb, new_blocks, new_block_cnt);
	update_fcb(_fcb, _user_open->f_fcb->attribute, new_block_size, _user_open->f_fcb->file_count, 0);
}

/**
 * 从指定fcb中读取size个字节的数据
 * @param sb 超级块
 * @param fcb 文件控制块
 * @param size 0表示读取整个文件
 * @return
 */
void* do_read(super_block* sb, fcb* fcb, size_t size)
{
	size_t rest_size = size;
	size_t* blocks;
	void* buff;
	rest_size = size == 0 ? fcb->length : size;
	setbuf(stdout, NULL);
	buff = malloc(rest_size);
	blocks = get_blocks(sb, fcb);
	size_t block_cnt = (rest_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
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
void do_write(super_block* sb, fcb* fcb, void* buff, size_t size)
{

	size_t* blocks, * new_blocks;
	size_t rest_size = size;
	size_t block_cnt = (fcb->length + BLOCK_SIZE - 1) / BLOCK_SIZE;
	blocks = get_blocks(sb, fcb);
	new_blocks = (size_t*)malloc((fcb->length + size + BLOCK_SIZE - 1) / BLOCK_SIZE * sizeof(size_t));
	memcpy(new_blocks, blocks, sizeof(size_t) * block_cnt);
	if (block_cnt > 0)
	{
		memcpy(index_to_addr(sb, new_blocks[block_cnt - 1]) + fcb->length % BLOCK_SIZE, buff,
			BLOCK_SIZE - fcb->length % BLOCK_SIZE);
		rest_size -= BLOCK_SIZE - fcb->length % BLOCK_SIZE;
	}
	for (size_t i = block_cnt; i < (fcb->length + size + BLOCK_SIZE - 1) / BLOCK_SIZE; i++)
	{
		new_blocks[i] = allocate_block(sb, 1);
		memcpy(index_to_addr(sb, new_blocks[i]), buff + size - rest_size,
			rest_size > BLOCK_SIZE ? BLOCK_SIZE : rest_size);
		rest_size -= BLOCK_SIZE;
	}
	save_blocks(sb, fcb, new_blocks, (fcb->length + size + BLOCK_SIZE - 1) / BLOCK_SIZE);
	//  先保存blocks，再更新fcb大小
	update_fcb(fcb, fcb->attribute, fcb->length + size, fcb->file_count, 0);

}

/**
 * 在超级块的索引节点表中申请一个空闲的索引节点
 * @param fcb
 * @return 节点编号
 */
ssize_t apply_inode(super_block* fcb)
{
	for (ssize_t i = 0; i < INODE_MAX_COUNT; i++)
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
void save_blocks(super_block* sb, fcb* fcb, size_t* blocks, size_t block_cnt)
{
	ssize_t old_block_cnt = (fcb->length + BLOCK_SIZE - 1) / BLOCK_SIZE;
	size_t* old_blocks = get_blocks(sb, fcb);
	if (block_cnt <= old_block_cnt)
	{
		// 先释放多余磁盘块
		for (size_t i = block_cnt; i < old_block_cnt; i++)
		{
			free_block(sb, old_blocks[i], 1);
		}
	}
	memcpy(fcb->mixed_index_block, blocks,
		sizeof(size_t) * (block_cnt < LEVEL0_BLOCK_CNT ? block_cnt : LEVEL0_BLOCK_CNT));
	if (block_cnt > LEVEL0_BLOCK_CNT)
	{
		if (old_block_cnt <= LEVEL0_BLOCK_CNT)
		{
			fcb->mixed_index_block[LEVEL0_BLOCK_CNT] = allocate_block(sb, 1);
		}
		memcpy(index_to_addr(sb, fcb->mixed_index_block[LEVEL0_BLOCK_CNT]),
			blocks + LEVEL0_BLOCK_CNT, sizeof(size_t) *
				(block_cnt < LEVEL0_BLOCK_CNT + LEVEL1_BLOCK_CNT ? block_cnt - LEVEL0_BLOCK_CNT : LEVEL1_BLOCK_CNT));
	}
	if (block_cnt > LEVEL0_BLOCK_CNT + LEVEL1_BLOCK_CNT)
	{
		if (old_block_cnt <= LEVEL0_BLOCK_CNT + LEVEL1_BLOCK_CNT)
		{
			fcb->mixed_index_block[LEVEL0_BLOCK_CNT + 1] = allocate_block(sb, 1);
		}
		size_t* level1_blocks = index_to_addr(sb, fcb->mixed_index_block[LEVEL0_BLOCK_CNT + LEVEL1_INDEX_CNT]);
		size_t level1_block_cnt =
			(block_cnt - LEVEL0_BLOCK_CNT - LEVEL1_BLOCK_CNT + LEVEL1_BLOCK_CNT - 1) / LEVEL1_BLOCK_CNT;
		ssize_t old_level1_block_cnt = old_block_cnt <= LEVEL0_BLOCK_CNT + LEVEL1_BLOCK_CNT ? 0 :
									   (old_block_cnt - LEVEL0_BLOCK_CNT - LEVEL1_BLOCK_CNT + LEVEL1_BLOCK_CNT - 1)
										   / LEVEL1_BLOCK_CNT;
		if (level1_block_cnt > old_level1_block_cnt)
			for (size_t i = old_level1_block_cnt; i < level1_block_cnt; i++)
				level1_blocks[i] = allocate_block(sb, 1);
		for (size_t i = 0; i < level1_block_cnt; i++)
			memcpy(index_to_addr(sb, level1_blocks[i]),
				blocks + LEVEL0_BLOCK_CNT + LEVEL1_BLOCK_CNT + i * LEVEL1_BLOCK_CNT,
				sizeof(size_t) * (block_cnt < LEVEL0_BLOCK_CNT + LEVEL1_BLOCK_CNT + (i + 1) * LEVEL1_BLOCK_CNT ?
								  block_cnt - LEVEL0_BLOCK_CNT - LEVEL1_BLOCK_CNT - i * LEVEL1_BLOCK_CNT
																											   : LEVEL1_BLOCK_CNT));
		memcpy(index_to_addr(sb, fcb->mixed_index_block[LEVEL0_BLOCK_CNT + 1]),
			level1_blocks, sizeof(size_t) * level1_block_cnt);
	}

}

/**
 * @param fcb 文件控制块
 * 更新文件fcb，该操作会更新文件的修改时间
 * @param attribute 属性
 * @param length 文件大小
 */
void update_fcb(fcb* fcb, unsigned char attribute, size_t length, size_t file_cnt, unsigned char create)
{
	fcb->attribute = attribute;
	fcb->length = length;
	fcb->file_count = file_cnt;
	time_t now = time(NULL);
	struct tm* t = localtime(&now);
	memcpy(&fcb->last_modify_time, t, sizeof(struct tm));
	if (create)
		memcpy(&fcb->create_time, t, sizeof(struct tm));
}

/**
 * 根据索引节点编号获取文件控制块
 * @param sb 超级块
 * @param index 索引节点编号
 * @return fcb指针
 */
fcb* index_to_fcb(super_block* sb, size_t index)
{
	return &sb->fcb_array[index];
}

void my_cat(super_block* sb, fcb* fcb)
{
	char* buf = malloc(fcb->length + 1);
	buf = do_read(sb, fcb, 0);
	for (size_t i = 0; i < fcb->length; i++)
		printf("%c", buf[i]);
}

/**
 * 创建目录文件  包括向目录文件中写入.和..两个目录项
 * @param sb  超级块
 * @param dir 上级目录
 * @param name 目录名
 * @return 创建的inode
 */
ssize_t create_dir(super_block* sb, fcb* dir, char* filename)
{
	ssize_t index = do_create_file(sb, dir, filename, DIRECTORY, 0);
	if (index < 0) return index;
	fcb* fcb = index_to_fcb(sb, index);
	fcb->file_count = 2;
	size_t dir_index[2];
	inode dir_inode[2];
	dir_index[0] = index;
	dir_index[1] = (*(inode*)do_read(sb, dir, sizeof(inode))).inode_index;
	strcpy(dir_inode[0].filename, ".");
	strcpy(dir_inode[1].filename, "..");
	dir_inode[0].attribute = DIRECTORY;
	dir_inode[1].attribute = DIRECTORY;
	dir_inode[0].inode_index = dir_index[0];
	dir_inode[1].inode_index = dir_index[1];
	do_write(sb, fcb, (char*)dir_inode, 2 * sizeof(inode));
	return index;
}
/**
 * 创建文件并写入指定内容
 * @param sb 超级块
 * @param dir 上级目录
 * @param filename 文件名
 * @param size 文件大小
 * @param content 文件内容
 * @return 创建的inode
 */
ssize_t create_file(super_block* sb, fcb* dir, char* filename, size_t size, void* content)
{
	ssize_t index = do_create_file(sb, dir, filename, ORDINARY_FILE, 0);
	if (index < 0) return index;
	fcb* fcb = index_to_fcb(sb, index);
	if (size > 0)
		do_write(sb, fcb, content, size);
	return index;
}

ssize_t delete_file(super_block* sb, fcb* fcb, struct FCB* dir)
{
//    todo
	if (fcb->attribute == DIRECTORY)
	{
		if (fcb->file_count > 2)
		{
			printf("directory is not empty\n");
			return ERR_PARAM_INVALID;
		}
		else
		{
			fcb->file_count = 0;
			fcb->length = 0;
			fcb->attribute = 0;
			dir->file_count--;
			return 0;
		}
	}
}

void clear_file(super_block* sb, fcb* fcb)
{
//    todo

}

/**
 * 根据目录文件的fcb获取目录文件的索引节点号
 * @param sb 超级块
 * @param fcb 文件控制块
 * @return 目录文件的索引节点号
 */
ssize_t dir_fcb_to_index(super_block* sb, fcb* fcb)
{
	if (fcb->attribute != DIRECTORY)
		return ERR_PARAM_INVALID;
	inode* inodes = (inode*)do_read(sb, fcb, sizeof(inode));
	ssize_t index = inodes[0].inode_index;
	free(inodes);
	return index;
}

int my_rmdir(super_block* sb,char **args){
    return 1;
}

int my_create(super_block* sb,char **args){
    return 1;
}

int my_rm(super_block* sb,char **args){
    return 1;
}

int my_exit_sys(super_block* sb,char **args){
    return 1;
}

int do_close(super_block* sb, char* filePath){
    fcb *fcb = findFcb(sb,filePath);

    for (int i = 0; i < MAX_OPEN_FILE; i++) {
        if (open_file_list[i].is_empty == 0) {
            if (fcb == open_file_list[i].f_fcb) {
                /*
                 * 复原对应open_file_list项
                 */
                return 1;
            }
        }
    }
}

int my_close(super_block* sb,char **args){
    /*
     * 相应工作然后调用close
     */
    return 1;
}