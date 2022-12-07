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
 * @return 找到返回对应的下标，满了返回ERR_FILE_NOT_OPENED
 */
int get_user_open()
{
	int i;
	for (i = 0; i < MAX_OPEN_FILE; i++)
	{
		if (!open_file_list[i])
		{
			open_file_list[i] = malloc(sizeof(user_open));
			return i;
		}
	}
	return ERR_FILE_NOT_OPENED;
}
/**
 * 根据文件路径和打开模式打开文件，加入open_file_list中
 * @param sb
 * @param filePath
 * @return open_file_list 的下标
 */
int do_open(super_block* sb, char* filePath)
{
	int fd = get_user_open();
	if (fd == ERR_FILE_NOT_OPENED)
	{
		printf("\"open error\": The number of file openings is maximum\n");
		return -1;
	}
	open_file_list[fd]->f_fcb = findFcb(sb, filePath);
	getFullPath(open_file_list[fd]->path, filePath);
	return fd;
}

/**
 * 打开单个文件或者目录文件 ，-l查看所有打开的文件
 * @param sb
 * @param args
 * @return
 */
int my_open(super_block* sb, char** args)
{
	int fd;
	//参数为空
	if (args[1] == NULL)
	{
		printf("open: missing argument!\n");
		return 1;
	}

	//查看帮助文档
	if (!strcmp(args[1], "--help"))
	{
		printf("Usage1: open [OPTION]...\n");
		printf("Usage2: open [FILE]...\n");
		printf("Open the files or list all the information of openfile \n");
		printf("OPTION:\n");
		printf("    -l: List all the information of openfile\n");
		return 1;
	}

	// -l查看所有已经打开的文件
	if (args[1][0] == '-')
	{
		if (!strcmp(args[1], "-l"))
		{
			do_printf(NULL,FILE_ATTRIBUTES_TITLE);
			for (int i = 0; i < MAX_OPEN_FILE; i++)
			{
				if (open_file_list[i])
				{
					do_printf(open_file_list[i]->f_fcb,FILE_ATTRIBUTES);
				}
			}
			return 1;
		}
		else if (strcmp(args[1], "--help"))
		{
			printf("\"open error\": wrong argument\n");
			return 1;
		}
	}

	int cnt = 1;
	char** p = args;
	for (p++; *p != NULL; p++)
	{
		char* filePath = NULL;
		if (*p != NULL)
		{
			filePath = *p;
		}
		else
		{
			printf("\"open error in argument%d\": missing path argument\n", cnt);
			return 1;
		}

		if (findFcb(sb, filePath) == NULL)
		{
			printf("\"open error in argument%d\": cannot open %s: There is no such file or folder\n", cnt, *p);
			return 1;
		}

		//如果当前文件已经被打开
		if (is_file_open(filePath) != -1)
		{
			printf("\"open error in argument%d\": cannot open %s: File or folder is open\n", cnt, filePath, *p);
			return 1;
		}

		//可以打开一个目录文件，但并不会更改当前工作目录和当前文件打开文件描述符。
		do_open(sb, filePath);
		cnt++;
	}


	return 1;
}

int is_file_open(char* filePath)
{
	char* fullPath = malloc(sizeof(char) * FILENAME_LEN);
	getFullPath(fullPath, filePath);
	for (int i = 0; i < MAX_OPEN_FILE; i++)
	{
		if (open_file_list[i])
		{
			if (strcmp(open_file_list[i]->path, fullPath) == 0)
			{
				return i;
			}
		}
	}
	return -1;
}

/**
 * 打印对应目录下的的所有文件
 * @param sb
 * @param filePath
 * @return
 */
int my_ls(super_block* sb, char** args)
{
	char** p = args;
	for (++p; *p != NULL; p++)
	{
		if (!strcmp(*p, "--help"))
		{
			printf("Usage: ls [OPTION]... [FILE]\n");
			printf("Lists the file information in the current directory \n");
			printf("OPTION:\n");
			printf("    -l: Lists the detailed file information in the current directory\n");
			return 1;
		}
		else if (!strcmp(*p, "-l"))
		{
			fcb* dirFcb;
			p++;
			if (*p == NULL)
			{
				dirFcb = current_dir;
			}
			else
			{
				dirFcb = findFcb(sb, *p);
				if (dirFcb == NULL)
				{
					printf("\"ls\": cannot ls %s: No such file or folder\n", *p++);
					return 1;
				}
				if (dirFcb->attribute == ORDINARY_FILE)
				{
					printf("\"ls\": ls cannot be used for files\n", *p++);
					return 1;
				}
			}
			do_printf(NULL,FILE_ATTRIBUTES_TITLE);
			inode* ptrInode = (inode*)do_read(sb, dirFcb, 0);
			for (int i = 0; i < dirFcb->file_count; ++i)
			{
				fcb* ptr = index_to_fcb(sb, ptrInode[i].inode_index);
				do_printf(ptr,FILE_ATTRIBUTES);
			}
			return 1;
		}
		else
		{
			break;
		}
	}

	fcb* dirFcb;
	if (*p == NULL)
	{
		dirFcb = current_dir;
	}
	else
	{
		dirFcb = findFcb(sb, *p);
		if (dirFcb == NULL)
		{
			printf("\"ls\": cannot ls %s: No such file or folder\n", *p++);
			return 1;
		}
		if (dirFcb->attribute == ORDINARY_FILE)
		{
			printf("\"ls\": ls cannot be used for files\n", *p++);
			return 1;
		}
	}
	inode* ptrInode = (inode*)do_read(sb, dirFcb, 0);
	for (int i = 0; i < dirFcb->file_count; ++i)
	{
		fcb* ptr = index_to_fcb(sb, ptrInode[i].inode_index);
		printf("%s\t", ptr->filename);
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
	char** p = args;
	int fd;
	for (++p; *p != NULL; p++)
	{
		if (!strcmp(*p, "--help"))
		{
			printf("Usage: cd [FILE]\n");
			printf("Change the current working directory \n");
			return 1;
		}
		else if (*(++p) == NULL)
		{
			char* filePath = NULL;
			if (*(--p) != NULL)
			{
				filePath = *p;
			}
			if (filePath == NULL)
			{
				filePath = (char*)malloc(sizeof(char) * _MAX_PATH);
				strcpy(filePath, current_dir_name);
			}

			getFullPath(filePath, filePath);
			//cd 到当前目录
			if (!strcmp(filePath, current_dir_name))
			{
				return 1;
			}

			fcb* fcb = findFcb(sb, filePath);
			//找不到这个文件
			if (fcb == NULL)
			{
				printf("\"cd\" error: cannot open %s: No such folder\n", filePath);
				return 1;
			}
			//不能cd到一个文件
			if (fcb->attribute == ORDINARY_FILE)
			{
				printf("\"cd\" error: cannot open %s: It is a file!\n", filePath);
				return 1;
			}

			char* old_current_dir_name = malloc(sizeof(char*) * FILENAME_LEN);
			strcpy(old_current_dir_name, current_dir_name);

			// 如果文件未打开，需要先打开这个文件然后再cd过去
			fd = is_file_open(filePath);
			current_dir_fd = fd == -1 ? do_open(sb, filePath) : fd;
			current_dir = fcb;
			getFullPath(current_dir_name, filePath);

			//关闭旧的目录文件
			do_close(sb, old_current_dir_name);
			free(old_current_dir_name);

			return 1;
		}
		else
		{
			printf("\"cd\" error: Too many parameters\n");
			return 1;
		}
	}


	return 1;
}

int my_mkdir(super_block* sb, char** args)
{
	if (args[1] == NULL)
	{
		printf("Dir name is not set.");
		return 1;
	}

	fcb* dirFcb = current_dir;
	inode* p_inode = (inode*)do_read(sb, dirFcb, 0);
	char** p = args;
	for (p++; *p != NULL; p++)
	{
		for (int i = 0; i < dirFcb->file_count; ++i)
		{
			fcb* _fcb = index_to_fcb(sb, p_inode[i].inode_index);
			if (!strcmp(_fcb->filename, *p))
			{
				printf("There is duplicate name file in current dir.");
				return 1;
			}
		}
		create_dir(sb, current_dir, *p);
	}
	return 1;
}

int my_pwd(super_block* sb, char** args)
{
	char** p = args;
	p++;

	if (*p == NULL)
	{
		printf("%s\n", current_dir_name);
		return 1;
	}
	if (!strcmp(*p, "--help"))
	{
		printf("Usage: pwd \n");
		printf("View the current working path \n");
	}
	else
	{
		printf("Unknown argument\n");
	}
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
 * @return 1
 */
int my_read(super_block* sb, user_open* _user_open, void* buf, size_t size)
{
	if (_user_open->f_fcb->attribute == DIRECTORY)
	{
		printf("File to read is not a file!\n");
		return 1;
	}
	if (!buf)
	{
		printf("Buffer is NULL!\n");
		return 1;
	}

	fcb* fcb = _user_open->f_fcb;
	size_t rest_size = fcb->length > size ? size : fcb->length;
	size_t offset = _user_open->p_WR;
	size_t* blocks = get_blocks(sb, fcb);
	size_t block_cnt = (fcb->length + BLOCK_SIZE - 1) / BLOCK_SIZE;

	_user_open->p_WR = 0;
	for (size_t i = 0; i < block_cnt; i++)
	{
		memcpy(buf + i * BLOCK_SIZE, index_to_addr(sb, blocks[i]), rest_size > BLOCK_SIZE ? BLOCK_SIZE : rest_size);
		rest_size -= BLOCK_SIZE;
		_user_open->p_WR += BLOCK_SIZE;
	}
	free(blocks);
	_user_open->p_WR = offset;
	return 1;
}

int my_write(super_block* sb, char** args)
{
	if (args[1] == NULL)
	{
		printf("Please input the file name.\n");
		return 1;
	}

	int mode = APPEND; // 默认追加模式
	int offset = 0;
	user_open* _user_open = NULL;

	// 写入模式
	if (args[2])
	{
		if (!strcmp(args[2], "-a"))
		{
			mode = APPEND;
		}
		else if (!strcmp(args[2], "-t"))
		{
			mode = TRUNCATE;
		}
		else if (!strcmp(args[2], "-o"))
		{
			if (args[3] == NULL)
			{
				printf("Argument is missing for mode OVERRIDE.\n");
				return 1;
			}
			else
			{
				offset = atoi(args[3]);
			}
			mode = OVERRIDE;
		}
	}

	// 从文件打开列表定位待写入文件
	for (int i = 0; i < open_file_count; i++)
	{
		if (!strcmp(open_file_list[i]->f_fcb->filename, args[1]))
		{
			if (open_file_list[i]->f_fcb->attribute == DIRECTORY)
			{
				printf("File trying to write is not a directory!\n");
				return 1;
			}
			_user_open = open_file_list[i];
			break;
		}
	}
	if (!_user_open)
	{
		printf("File not opened!\n");
		return 1;
	}
	_user_open->mode = mode;
	char* buf = do_read_ch(stdin);
	_do_write(sb, _user_open, buf, sizeof(buf));

	return 1;
}


/**
 * 从输入流读取字符序列，直到遇到结束符EOF
 * @param stream 输入流
 * @return 字符串void*
 */

// 从输入流读取字符序列，直到遇到结束符EOF
void* do_read_ch(void* stream)
{
	getchar();
	const int BUFFER_BASE = 512;
	int buf_size = BUFFER_BASE;
	char* buf = malloc(buf_size * sizeof(char));
	char ch;
	int i = 0;
	while ((ch = fgetc(stream)) != EOF)
	{
		if (i + 1 >= buf_size)
		{
			buf_size += BUFFER_BASE;
			void* old_buf = buf;
			char* new_buf = malloc(buf_size * sizeof(char));
			memcpy(new_buf, old_buf, i * sizeof(char));
			free(old_buf);
			buf = new_buf;
		}
		buf[i++] = ch;
	}
	buf[i] = '\0';

	char* final_buf = malloc(i * sizeof(char));
	memcpy(final_buf, buf, i * sizeof(char));
	free(buf);
	return final_buf;
}

/**
 * 写入文件
 * @param sb 超级块
 * @param _user_open 待写入文件的user_open
 * @param buf 待写入的文件内容
 * @param size 写入的长度
 * @return
 */
void _do_write(super_block* sb, user_open* _user_open, void* buf, size_t size)
{
	fcb* _fcb = _user_open->f_fcb;
	size_t* old_blocks, * new_blocks;
	size_t old_block_cnt = (_fcb->length + BLOCK_SIZE - 1) / BLOCK_SIZE;
	size_t old_block_frag = _fcb->length % BLOCK_SIZE; //原始文件最后一块的块大小
	size_t new_block_size = 0;
	size_t new_block_cnt = 0;
	size_t rest_size = size;
	size_t size_to_write = 0;
	int mode = _user_open->mode;

	switch (mode & 0b111)
	{
	case APPEND:
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
		for (size_t i = old_block_cnt; i < new_block_cnt; i++)
		{
			new_blocks[i] = allocate_block(sb, 1);
			size_to_write = rest_size > BLOCK_SIZE ? BLOCK_SIZE : rest_size;
			memcpy(index_to_addr(sb, new_blocks[i]), buf + size - rest_size, size_to_write);
			rest_size -= size_to_write;    // do_write中减去了BLOCK_SIZE
			_user_open->p_WR += size_to_write;
		}
		break;
	case TRUNCATE:
		_user_open->p_WR = 0;
		free_block(sb, addr_to_index(sb, (void*)_user_open->f_block_start), old_block_cnt);
		new_block_size = size;
		new_block_cnt = (new_block_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
		for (size_t i = 0; i < new_block_cnt; i++)
		{
			new_blocks[i] = allocate_block(sb, 1);
			size_to_write = rest_size > BLOCK_SIZE ? BLOCK_SIZE : rest_size;
			memcpy(index_to_addr(sb, new_blocks[i]), buf + size - rest_size, size_to_write);
			rest_size -= size_to_write;
			_user_open->p_WR += size_to_write;
		}
		break;
	case OVERRIDE:
		new_block_size = size > (_fcb->length - _user_open->p_WR) ? _user_open->p_WR + size : size;
		new_block_cnt = (new_block_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
		new_blocks = (size_t*)malloc(sizeof(size_t) * new_block_cnt);
		for (size_t i = 0; i < new_block_cnt; i++)
		{
			new_blocks[i] = allocate_block(sb, 1);
			size_to_write = rest_size > BLOCK_SIZE ? BLOCK_SIZE : rest_size;
			memcpy((void*)_user_open->p_WR, buf + size - rest_size, size_to_write);
			rest_size -= size_to_write;
			_user_open->p_WR += size_to_write;
		}
		break;
	default:
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
 * 更新文件fcb，该操作会更新文件的修改时间
 * @param fcb 文件控制块
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
	fcb->attribute = DIRECTORY;
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

/**
 * 从dir目录中删除fcb文件
 * @param sb 超级块
 * @param fcb 文件
 * @param dir 目录
 * @return 0成功  <0错误
 */
ssize_t delete_file(super_block* sb, fcb* fcb, struct FCB* dir)
{
	if (fcb->attribute == DIRECTORY)
	{
		if (strcpy(fcb->filename, "/") == 0)
		{
			printf("Not allowed to delete root dir\n");
			return ERR_PARAM_INVALID;
		}
		else if (fcb->file_count > 2)
		{
			printf("directory is not empty\n");
			return ERR_PARAM_INVALID;
		}
		else
		{
			// 释放文件blocks,把fcb的is_used置为0（在索引节点表中删除），在dir的文件内容中删除该文件的inode
            size_t *blocks = get_blocks(sb, fcb);
            size_t block_cnt = (fcb->length + BLOCK_SIZE - 1) / BLOCK_SIZE;
            for (size_t i = 0; i < block_cnt; i++) {
                free_block(sb, blocks[i], 1);
            }
            free(blocks);
            fcb->is_used = 0;
            inode *inodes = (inode*)do_read(sb, dir, 0);
//          从目录文件内删除这一项
            for (size_t i = 0; i < dir->file_count; i++) {
                if (strcpy(inodes[i].filename, fcb->filename) == 0) {
                    for (size_t j = i; j < dir->file_count - 1; j++) {
                        memcpy(&inodes[j], &inodes[j + 1], sizeof(inode));
                    }
                    break;
                }
            }
            dir->file_count--;
            clear_file(sb, dir);
            do_write(sb, dir, (char*)inodes, dir->file_count * sizeof(inode));
            free(inodes);
            return 0;
		}
	}
}

/**
 * 清除文件内容，目录文件不会清除..和.目录项
 * @param sb
 * @param fcb
 */
void clear_file(super_block* sb, fcb* fcb)
{
    size_t* blocks = get_blocks(sb, fcb);
    size_t block_cnt = (fcb->length + BLOCK_SIZE - 1) / BLOCK_SIZE;
    inode *inodes;
    if (fcb->attribute == DIRECTORY)
    {
        inodes = (inode*)do_read(sb, fcb, sizeof(inode) * 2);
    }
    for (size_t i = 0; i < block_cnt; i++)
    {
        free_block(sb, blocks[i], 1);
    }
    free(blocks);
    if (fcb->attribute == ORDINARY_FILE)
        update_fcb(fcb, fcb->attribute, 0, 0, 0);
    else {
        update_fcb(fcb, fcb->attribute, 0, 2, 0);
        do_write(sb, fcb, (char*)inodes, sizeof(inode) * 2);
        free(inodes);
    }
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

int my_rmdir(super_block* sb, char** args)
{
	return 1;
}

int my_create(super_block* sb, char** args)
{
	return 1;
}

int my_rm(super_block* sb, char** args)
{
	return 1;
}

int my_exit_sys(super_block* sb, char** args)
{
    save(DISK_BACKUP_FILENAME, sb, SIZE);
    exit(0);
}

int do_close(super_block* sb, char* filePath)
{
	if (findFcb(sb, filePath) == NULL)
	{
		printf("\"close error\": cannot open %s: There is no such file or folder\n", filePath);
		return 1;
	}
	int index = is_file_open(filePath);
	if (index != -1)
	{
		//	当前工作路径无法close
		if (!strcmp(open_file_list[index]->path, current_dir_name))
		{
			printf(
				"\"close\" error: cannot close %s: The current working path cannot be closed, please exit the directory first\n",
				filePath);
			return -1;
		}
		else
		{
			free(open_file_list[index]);
			open_file_list[index] = NULL;
			return 0;
		}

	}
	else
	{
		printf("File is not open.\n");
		return -1;
	}
}

int my_close(super_block* sb, char** args)
{
	char** p = args;
	for (p++; *p != NULL; p++)
	{
		do_close(sb, *p);
	}
	return 1;
}

int my_touch(super_block* sb, char** args)
{
	if (args[1] == NULL)
	{
		printf("touch: missing file operand\n");
		return 1;
	}

	char** p = args;
	for (p++; *p != NULL; p++)
	{
		if (create_file(sb, current_dir, *p, 0, 0) < 0)
		{
			printf("touch: cannot create file %s.\n", *p);
			return 1;
		}
	}
	return 1;
}

void do_printf(fcb* ptr,int format){
	switch (format)
	{
	case FILE_ATTRIBUTES_TITLE:
		printf("%-10s\t%-10s\t%-12s\t%-16s\t%-16s\n",
			"filename",
			"length",
			"attribute",
			"create time",
			"last modify time");
		break;
	case FILE_ATTRIBUTES:
		if(!ptr){
			printf("fcb NULL");
			return;
		}
		printf("%-10s\t%-10d\t%-12s\t%-04d-%02d-%02d %02d:%02d\t%04d-%02d-%02d %02d:%02d\t\n",
			ptr->filename,
			ptr->length,
			ptr->attribute == 1 ? "directory" : "file",
			ptr->create_time.tm_year + BASE_YEAR,
			ptr->create_time.tm_mon,
			ptr->create_time.tm_mday,
			ptr->create_time.tm_hour,
			ptr->create_time.tm_min,
			ptr->last_modify_time.tm_year + BASE_YEAR,
			ptr->last_modify_time.tm_mon,
			ptr->last_modify_time.tm_mday,
			ptr->last_modify_time.tm_hour,
			ptr->last_modify_time.tm_min);
		break;
	}
}
