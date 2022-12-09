#include <sys/types.h>
#include "myfs.h"
// 初始化文件系统
// 首先检查文件系统备份文件是否存在
// 从备份文件恢复或者格式化一个新的文件系统
// 所有的指针都要根据偏差重新修正

void recover(super_block** sb, char* bak_file)
{
	FILE* fp;
	void* fs;
	ptrdiff_t delta;
	fopen_s(&fp, bak_file, "rb");
	fs = malloc(SIZE);
	if (fs == NULL)
	{
		printf("malloc error\n");
		exit(1);
	}
	size_t cnt = fread(fs, 1, SIZE, fp);
	if (cnt != SIZE)
	{
		perror("fread error");
	}
	*sb = (super_block*)fs;

	delta = (char*)fs - (char*)(*sb)->start_pos;
	(*sb)->start_pos = fs;
	free_block_list* p_fbl = &(*sb)->free_block_list;
	update_list(p_fbl, list, delta)
	(*sb)->fcb_array = (void*)(*sb)->fcb_array + delta;
	fclose(fp);

	current_dir = index_to_fcb(*sb, (*sb)->root_index);
    do_open(*sb, current_dir_name);
}

// 传递超级块指针的指针是为了让超级块指针指向0号内存
// 也就是将超级块置于文件系统的起始位置
void start_sys(char* bak_file, super_block** sb, int recreate)
{
	FILE* fp;
	fopen_s(&fp, bak_file, "rb");
	if (recreate)
	{
        free(*sb);
		printf("Recreating file system\n");
		my_format(sb);
		if (fp != NULL)
		{
			fclose(fp);
			remove(bak_file);
		}
        puts("File system created");
	}
	else if (fp == NULL)
	{
		printf("No backup file found, creating a new file system.\n");
		my_format(sb);
        puts("File system created");
	}
	else
	{
		printf("Backup file found, recovering file system.\n");
		fclose(fp);
		recover(sb, bak_file);
        puts("File system recovered");
	}
    show_fs_info(*sb);
}

// 格式化文件系统
// 包括初始化超级块、初始化空闲盘区链
void my_format(super_block** p_sb)
{
	void* fs = malloc(SIZE);
	free_block_list* fb_node;  // fb_node用于指向第一个空闲盘块

	if (fs == NULL)
	{
		printf("malloc error\n");
		exit(1);
	}
	// 把超级块指针指向0号地址空间
	*p_sb = (super_block*)fs;

	super_block* sb = (super_block*)fs;
	sb->start_pos = fs;
	sb->block_count = SIZE / BLOCK_SIZE;
	sb->free_block_count = (SIZE - sizeof(super_block)) / BLOCK_SIZE;
	sb->free_block_list_index = SIZE / BLOCK_SIZE - sb->free_block_count;
	INIT_LIST_HEAD(&sb->free_block_list.list);
	fb_node = (free_block_list*)(fs + sb->free_block_list_index * BLOCK_SIZE);
	fb_node->block_index = sb->free_block_list_index;
	fb_node->count = sb->free_block_count;
	list_add(&fb_node->list, &sb->free_block_list.list);

	size_t fcb_array_block_index = allocate_block(sb, (sizeof(fcb) * INODE_MAX_COUNT + BLOCK_SIZE - 1) / BLOCK_SIZE);
	if (fcb_array_block_index != 1)
	{
		printf("fcb_array_block_index error\n");
		exit(1);
	}
	sb->fcb_array = index_to_addr(sb, fcb_array_block_index);
	ssize_t fcb_index = do_create_file(sb, NULL, "/", DIRECTORY, 0);
	if (fcb_index < 0)
	{
		printf("create root directory error\n");
		exit(1);
	}
	sb->root_index = fcb_index;
	inode root_dot;
	root_dot.inode_index = fcb_index;
	root_dot.attribute = DIRECTORY;
	strcpy(root_dot.filename, ".");
	do_write(sb, index_to_fcb(sb, fcb_index), &root_dot, sizeof(inode));
	index_to_fcb(sb, fcb_index)->file_count = 1;
	index_to_fcb(sb, fcb_index)->length = sizeof(inode);
	size_t users_inode = create_dir(sb, index_to_fcb(sb, fcb_index), "users");
	create_dir(sb, index_to_fcb(sb, users_inode), "root");
	create_dir(sb, index_to_fcb(sb, users_inode), "guest");
	create_dir(sb, index_to_fcb(sb, sb->root_index), "groups");


    current_dir = index_to_fcb(sb, sb->root_index);
	// 初始化文件打开表
	do_open(sb, current_dir_name);
}

void save(char* bak_file, super_block *sb, size_t size)
{
	FILE* fp;
	size_t ret;
	void* start_pos = sb->start_pos;
	fopen_s(&fp, bak_file, "wb+");
	if (fp == NULL)
	{
		printf("open file error\n");
		exit(1);
	}
	ret = fwrite(start_pos, 1, size, fp);
	if (ret != size)
	{
		printf("write error\n");
		exit(1);
	}
//  保存索引节点数组
	ret = fwrite(sb->fcb_array, 1, sizeof(fcb) * INODE_MAX_COUNT, fp);
	if (ret != sizeof(fcb) * INODE_MAX_COUNT)
	{
		printf("write error\n");
		exit(1);
	}
	fclose(fp);
}

__attribute__((unused)) void show(free_block_list* fbl)
{
	free_block_list* p;
	list_for_each_entry(p, &fbl->list, list)
	{
		printf("block_index: %lld, count: %lld\n", p->block_index, p->count);
	}
}

void show_fs_info(super_block* sb)
{
	printf("%-20s: %-3lldMB %-3lldKB\n",
		"disk space size",
		sb->block_count * BLOCK_SIZE / 1024 / 1024,
		sb->block_count * BLOCK_SIZE / 1024 % 1024);
	printf("%-20s: %-3lldMB %-3lldKB\n",
		"free space size",
		sb->free_block_count * BLOCK_SIZE / 1024 / 1024 % 1024,
		sb->free_block_count * BLOCK_SIZE / 1024 % 1024);
	printf("%-20s: %-3lldKB\n", "block size", BLOCK_SIZE / 1024);
	printf("%-20s: %-3lldMB %-3lldKB \n",
		"max file size",
		MAX_FILE_SIZE / 1024 / 1024,
		MAX_FILE_SIZE / 1024 % 1024);

}

int getLine(char* str, int lim, FILE* f)
{
	char c;
	int i=0;
    while ((c = fgetc(f)) != EOF && c!='\n')
    {
        str[i++] = c;
    }
	// 处理输入时候的换行符
	str[i] = '\0';

	return i;
}

char** getArgs(char* cmd)
{
	int bufSize = MAX_ARG_LENGTH, position = 0;
	char** tokens = (char**)malloc(bufSize * sizeof(char*));
	char* token;

	if (!tokens)
	{
		printf("\"csh error\": allocation error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(cmd, CSH_TOK_DELIM);
	while (token != NULL)
	{
		tokens[position] = token;
		position++;

		if (position >= bufSize)
		{
			bufSize += MAX_ARG_LENGTH;
			tokens = realloc(tokens, bufSize * sizeof(char*));
			if (!tokens)
			{
				printf("\"csh error\": allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL, CSH_TOK_DELIM);
	}
	tokens[position] = NULL; //在字符串数组末尾添加NULL，方便遍历
	return tokens;
}

int execute(super_block* sb, char** args)
{
	if (args[0] == NULL)
		//空命令
		return 1;
	for (int i = 0; i < sizeof(cmd) / sizeof(*cmd); ++i)
	{
		if (!strcmp(args[0], cmd[i]))
		{
			return (*cmd_func[i])(sb, args);
		}
	}
	printf("\"csh error\": Unknown command\n");
    return 1;
}

void show_csh(super_block* sb)
{
    printf("Welcome to our file system!\nWould you like to recover from the backup file? (y/n)\n");
    char c;
    scanf("%c", &c);
    if (c == 'y' || c == 'Y')
    {
        start_sys(DISK_BACKUP_FILENAME, &sb, 0);
    }
    else
    {
        start_sys(DISK_BACKUP_FILENAME, &sb, 1);
    }
    getchar();
	char* cmd = malloc(MAX_CMD_LENGTH);
	char** args;
	int status = 1;
	do
	{
		printf("%s>", current_dir_name);
		getLine(cmd, MAX_CMD_LENGTH, stdin);
		args = getArgs(cmd);
		status = execute(sb, args);

/* 遍历参数
 * for (char **ptr =args; *ptr!=NULL; ptr++)
            printf("arg :%s\n",*ptr);*/
	} while (status);
}

int main()
{
    super_block* sb = NULL;
    setbuf(stdout, NULL);
	show_csh(sb);
	return 0;
}

