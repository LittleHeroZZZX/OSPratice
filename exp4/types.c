#include <malloc.h>
#include "types.h"
#include "file.h"

super_block* sb;

int open_file_count = 0;

fcb* current_dir;

user_open* open_file_list[MAX_OPEN_FILE];

int current_dir_fd;  //当前打开文件的文件描述符
char current_dir_name[_MAX_PATH] = "/";

//维护一个所有命令的数组
char cmd[15][6] = {
	"cd",
	"mkdir",
	"rmdir",
	"ls",
	"create",
	"rm",
	"cat",
	"write",
	"read",
	"exit",
	"open",
	"close",
	"pwd",
	"touch",
	"clear"
};

//维护一个函数地址指针的数组
int (* cmd_func[])(super_block* sb, char**) = {
	&my_cd,
	&my_mkdir,
	&my_rmdir,
	&my_ls,
	&my_create,
	&my_rm,
	&my_cat,
	&my_write,
	&my_read,
	&my_exit_sys,
	&my_open,
	&my_close,
	&my_pwd,
	&my_touch,
	&my_clear
};