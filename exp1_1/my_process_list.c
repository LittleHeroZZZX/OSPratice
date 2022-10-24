#include <linux/init.h>
#include <linux/init_task.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>

#define MAX_NAME_LENGTH 12

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zhou Xin");
MODULE_DESCRIPTION("list all threads");

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

static int list_process_init(void) {
  struct task_struct *p;
  struct pid *pid_struct;
  printk(KERN_WARNING "Hello Zhou Xin! I'm installed.\n");
  printk(KERN_WARNING "%12s%8s%15s%12s%8s", "Name", "PID", "State", "Priority",
         "Parent");
  pid_struct = find_get_pid(2);
  p = get_pid_task(pid_struct, PIDTYPE_PID);
  struct task_struct *child;
  list_for_each_entry(child, &p->children, sibling) {
    if (getLenth(child->comm) <= MAX_NAME_LENGTH)
      printk(KERN_WARNING "%12.12s%8d%15s%12d%8d", child->comm, child->pid,
             stateInt2Str(child->state), child->prio, child->parent->pid);
    else
      printk(KERN_WARNING "%11.11s+%8d%15s%12d%8d", child->comm, child->pid,
             stateInt2Str(child->state), child->prio, child->parent->pid);
  }
  return 0;
}

static void list_process_exit(void) {
  printk(KERN_WARNING "Bye Zhou Xin! I'm removed.\n");
  return;
}

module_init(list_process_init);
module_exit(list_process_exit);
