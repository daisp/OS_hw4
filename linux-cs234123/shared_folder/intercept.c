#include <linux/sched.h>
#include <stdbool.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/utsname.h>
#include <linux/syscall.h>
// TODO: add more #include statements as necessary

#define MAX_NAME_SIZE 16

MODULE_LICENSE("GPL");

// TODO: add command-line arguments
void **sys_call_table = NULL;

asmlinkage long (*original_sys_kill)(void);

char *program_name = NULL;
int scan_range = 0;
MODULE_PARM(program_name, "s");
MODULE_PARM(scan_range, "i");

/* New fake syscall: */
asmlinkage long our_sys_kill(int pid, int sig) {
    printk("******   ENTERING THE FAKE SYS_KILL!   ******\npid=%ld, sig=%d\n", (long)pid, sig);
    if(!pid) {
        printk("calling original sys_kill because !pid is true\n");
        return sys_kill(pid, sig);
    }
    task_t* p = find_task_by_pid(pid);
    if (strcmp(p->comm, program_name)==0 &&
        sig == SIGKILL) {
        printk("Ignoring SIGKILL\n");
        return -EPERM;
    }
    return sys_kill(pid, sig);
}

void find_sys_call_table(int scan_range) {
    void *p = &system_utsname;
    printk("system_utsname is at %p\nsys_read is at %p\n", &system_utsname, &sys_read);
    int i;
    bool found_flag = false;
    for (i=0; i < scan_range; p += sizeof(void*), i++) {
        if (*(unsigned int*)p == sys_read){
            found_flag = true;
            break;
        }
    }
    if (found_flag) printk("Found sys_read at %p\n",p);
    else printk("Did not find sys_read\n");
    p = p - 3*sizeof(void *); //sys_read syscall num is 3, so reduce the offset to get to syscall table
    sys_call_table = p;
    printk("sys_call_table variable is now %p\n", sys_call_table);
}

int init_module(void) {
    /* set sys_kill syscall to our fake syscall: */
    if (program_name == NULL){
        printk("program_name is NULL, doing nothing");
        return 0;
    }
    printk("=====================================\nscan_range is %d, program_name is %s\n", scan_range,program_name);
    find_sys_call_table(scan_range);
    original_sys_kill = sys_call_table[37]; //sys_kill num is 37
    sys_call_table[37] = our_sys_kill;
    return 0;
}

void cleanup_module(void) {
    /* restore original syscall: */
    sys_call_table[37] = original_sys_kill;
}

