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

static char* src_filename = NULL;
module_param(src_filename, charp, 0644);
MODULE_PARM_DESC(filename, "source file");

static char* dest_filename = NULL;
module_param(dest_filename, charp, 0644);
MODULE_PARM_DESC(filename, "destination file");

static int copy_file_init(void){
    struct file *fp_src, *fp_dest;

    fp_src = filp_open(src_filename, O_RDONLY, 0);
    if (IS_ERR(fp_src)){
        printk(KERN_ERR "open file %s failed!\n", src_filename);
        return -1;
    }
    fp_dest = filp_open(dest_filename, O_WRONLY | O_CREAT, 0644);
    if (IS_ERR(fp_dest)){
        printk(KERN_ERR "open file %s failed!\n", dest_filename);
        return -1;
    }
    char buf[1024];
    int len;
    printk(KERN_WARNING "read %d bytes ，content is %s\n", len, buf);
    while ((len = kernel_read(fp_src, buf, 1024, &fp_src->f_pos)) > 0){
        kernel_write(fp_dest, buf, len, &fp_dest->f_pos);
    }
    printk(KERN_WARNING "copy file %s to %s successfully!\n", src_filename, dest_filename);
    filp_close(fp_src, NULL);
    filp_close(fp_dest, NULL);
    return 0;
}

static void copy_file_exit(void){
    printk(KERN_INFO "copy file exit!\n");
}

module_init(copy_file_init);
module_exit(copy_file_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zhou Xin");
MODULE_DESCRIPTION("copy file");