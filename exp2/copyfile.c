/*
 * @Author: zhouxin zzzx@hdu.edu.cn
 * @Date: 2022-10-19 21:07:39
 * @LastEditTime: 2022-10-20 23:33:40
 * @Software: VSCode
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/init_task.h>
#include <linux/fs.h>

static char* src = NULL;
module_param(src, charp, 0644);
MODULE_PARM_DESC(filename, "source file");

static char* dest = NULL;
module_param(dest, charp, 0644);
MODULE_PARM_DESC(filename, "destination file");

static int copy_file_init(void){
    struct file *fp_src, *fp_dest;
    loff_t pos_src = 0, pos_dest = 0;
    mm_segment_t fs;
    int ret;

    fp_src = filp_open(src, O_RDONLY, 0);
    if (IS_ERR(fp_src)){
        printk(KERN_ERR "open file %s failed!\n", src);
        return -1;
    }
    fp_dest = filp_open(dest, O_WRONLY | O_CREAT, 0644);
    if (IS_ERR(fp_dest)){
        printk(KERN_ERR "open file %s failed!\n", dest);
        return -1;
    }
    pos_src = fp_src->f_pos;
    pos_dest = fp_dest->f_pos;
    char buf[1024];
    int len;
    set_fs(KERNEL_DS);
    while ((len = kernel_read(fp_src, buf, 1024, &pos_src)) > 0){
        kernel_write(fp_dest, buf, len, &pos_dest);
    }
    filp_close(fp_src, NULL);
    filp_close(fp_dest, NULL);
    set_fs(fs);
    return 0;
}
static void copy_file_exit(void){
    printk(KERN_WARNING "copy file exit!\n");
}

module_init(copy_file_init);
module_exit(copy_file_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zhou Xin");
MODULE_DESCRIPTION("copy file");