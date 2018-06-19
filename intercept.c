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
MODULE_PARM(program_name, "s");

bool compare_name(char *str1, char *str2) {
    for (int i = 0; i < MAX_NAME_SIZE; i++) {
        if (str1[i] != str2[i])
            return false;
    }
    return true;
}

/* New fake syscall: */
asmlinkage long our_sys_kill(int pid, int sig) {
    if (compare_names(find_task_by_pid(pid)->comm, program_name) &&
        sig == SIGKILL) {
        return -EPERM;
    }
    return sys_kill();
}

// TODO: import original syscall and write new syscall

void find_sys_call_table(int scan_range) {
    int *p = system_utsname;
    int i = 0;
    for (; i < scan_range; p++, i++) {
        if (p == sys_read)
            break;
    }
    p = p - 3; //sys_read syscall num is 3, so reduce the offset to get to syscall table
    sys_call_table = p;
}

int init_module(void) {
    /* set sys_kill syscall to our fake syscall: */
    original_sys_kill = sys_call_table[37]; //sys_kill num is 37
    sys_call_table[37] = our_sys_kill;
    return 0;
}

void cleanup_module(void) {
    /* restore original syscall: */
    sys_call_table[37] = original_sys_kill;
}

