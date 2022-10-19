/*
 * @Author: zhouxin zzzx@hdu.edu.cn
 * @Date: 2022-10-18 20:31:32
 * @LastEditTime: 2022-10-18 20:31:33
 * @Software: VSCode
 */
#include <linux/init.h>
#include <linux/init_task.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pid.h>
#include <linux/sched.h>

#define MAX_NAME_LENGTH 12

static int pid_number = -1;
module_param(pid_number, int, 0644);
MODULE_PARM_DESC(pid_number, "The PID of the process to be examined");
int getLenth(char *str) {
  int cnt = 0;
  while (*str++ != '\0')
    cnt++;
  return cnt;
}

char *stateInt2Str(int taskStateValue) {
  char *strState = NULL;
  switch (taskStateValue) {
  case TASK_RUNNING:
    strState = "RUNNING";
    break;
  case TASK_INTERRUPTIBLE:
    strState = "INTERRUPTIBLE";
    break;
  case TASK_UNINTERRUPTIBLE:

    strState = "UNINTERRUPTIBLE";
    break;
  case __TASK_STOPPED:
    strState = "STOPPED";
    break;
  case __TASK_TRACED:
    strState = "TRACED";
    break;
  case EXIT_DEAD:
    strState = "DEAD";
    break;
  case EXIT_ZOMBIE:
    strState = "ZOMBIE";
    break;
  case TASK_DEAD:
    strState = "DEAD";
    break;
  case TASK_WAKEKILL:
    strState = "WAKEKILL";
    break;
  case TASK_WAKING:
    strState = "WAKING";
    break;
  case TASK_PARKED:
    strState = "PARKED";
    break;
  case TASK_NOLOAD:
    strState = "NOLOAD";
    break;
  case TASK_NEW:
    strState = "NEW";
    break;
  case TASK_STATE_MAX:
    strState = "MAX";
    break;
  case TASK_KILLABLE:
    strState = "KILLABLE";
    break;
  case TASK_STOPPED:
    strState = "STOPPED";
    break;
  case TASK_TRACED:
    strState = "TRACED";
    break;
  case TASK_IDLE:
    strState = "IDLE";
    break;
  case TASK_NORMAL:
    strState = "NORMAL";
    break;
  default:
    strState = "UNKNOWN";
    break;
  }
  return strState;
}

void printProcess(struct task_struct *task, char* relationship) {
  if (getLenth(task->comm) <= MAX_NAME_LENGTH)
    printk(KERN_WARNING "%8s%12s%6d%12s",relationship, task->comm, task->pid, stateInt2Str(task->__state));
  else
    printk(KERN_WARNING "%8s%11s+%6d%12s",relationship, task->comm, task->pid, stateInt2Str(task->__state));

}


static int list_process_init(void) {
  printk(KERN_WARNING "Hello Zhou Xin! I'm installed.\n");
  // if (pid_number == -1) {
  //   printk(KERN_WARNING "No PID specified.\n");
  //   return 0;
  // }
  // struct task_struct *p;
  // struct pid *pid_struct;
  
  // printk(KERN_WARNING "%8s%12s%6s%12s", "Relationship", "Name", "PID", "STATE");
  // // print parent
  // pid_struct = find_get_pid(pid_number);
  // p = get_pid_task(pid_struct, PIDTYPE_PID);
  // printProcess(p, "Parent");
  // // print children
  // struct task_struct *child;
  // list_for_each_entry(child, &p->children, sibling) {
  //   printProcess(child, "Child");
  // }
  // // print sibling
  // struct task_struct *sibling;
  // list_for_each_entry(sibling, &p->sibling, sibling) {
  //   printProcess(sibling, "Sibling");
  // }
  return 0;
}

static void list_process_exit(void) {
  printk(KERN_WARNING "Bye Zhou Xin! I'm removed.\n");
  return;
}

module_init(list_process_init);
module_exit(list_process_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zhou Xin");
MODULE_DESCRIPTION("list all threads");
