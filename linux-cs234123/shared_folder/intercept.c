#include <linux/sched.h>
#include <stdbool.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/utsname.h>
#include <linux/syscall.h>
#include <linux/unistd.h>
// TODO: add more #include statements as necessary

#define MAX_NAME_SIZE 16
#define SCAN_RANGE 600

MODULE_LICENSE("GPL");

// TODO: add command-line arguments
void** sys_call_table = NULL;

typedef asmlinkage long (*fpointer)(void);
fpointer original_sys_kill;

char *program_name = NULL;
MODULE_PARM(program_name, "s");


/* New fake syscall: */
asmlinkage long our_sys_kill(int pid, int sig) {
    // printk("******   ENTERING THE FAKE SYS_KILL!   ******\npid=%ld, sig=%d\n", (long)pid, sig);
    task_t* p = find_task_by_pid(pid);
    if(!p) {
        // printk("calling original sys_kill because p is null\n");
        return sys_kill(pid, sig);
    }
    if (strncmp(p->comm, program_name, 16)==0 &&
        sig == SIGKILL) {
        // printk("Ignoring SIGKILL\n");
        return -EPERM;
    }
    return sys_kill(pid, sig);
}

void find_sys_call_table(int scan_range) {
    fpointer *p = (fpointer *)&system_utsname;
    // printk("system_utsname is at %p\n", &system_utsname);
    int i;
    // bool found_flag = false;
    for (i=0; i < scan_range; p++, i++) {
        // printk("looking at address %p\n", p);
        if (*p == (fpointer)sys_read){
            // found_flag = true;
            break;
        }
    }
    // if (found_flag) printk("Found sys_read at %p\n",p);
    // else printk("Did not find sys_read\n");
    p = p - __NR_read; //sys_read syscall num is 3, so reduce the offset to get to syscall table
    sys_call_table = (void**)p;
    // printk("sys_call_table variable is now %p\n", sys_call_table);
}

int init_module(void) {
    /* set sys_kill syscall to our fake syscall: */
    if (program_name == NULL){
        // printk("program_name is NULL, doing nothing\n");
        return 0;
    }
    // printk("=====================================\nprogram_name is %s\n",program_name);
    find_sys_call_table(SCAN_RANGE);
    original_sys_kill = (fpointer)sys_call_table[__NR_kill]; //sys_kill num is 37=__NR_kill
    sys_call_table[__NR_kill] = (fpointer)our_sys_kill;
    return 0;
}

void cleanup_module(void) {
    /* restore original syscall: */
    if (sys_call_table == NULL){
        return;
    }
    sys_call_table[__NR_kill] = (unsigned int*)original_sys_kill;
}

