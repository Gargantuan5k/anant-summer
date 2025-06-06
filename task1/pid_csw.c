#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Siddharth Vivek");
MODULE_DESCRIPTION("A basic module that prints the parent process ID and the number of context switches");

static int __init drv_init(void)
{
    if (current->parent)
    {
        msleep(10);
        printk("csw Before pid printing: %lu\n", current->nvcsw + current->nivcsw);
        printk(KERN_INFO "current process '%s' (PID %d) loaded by parent process '%s' (PID %d)\n", current->comm, current->pid, current->parent->comm, current->parent->pid);
    }
    else
    {
        printk(KERN_ERR "could not get parent info\n");
        return 1;
    }

    return 0;
}

static void __exit drv_exit(void)
{
    int i;

    for (i = 0; i < 5; i++)
    {
        printk(KERN_INFO "random message %d\n", i);
        msleep(10);
    }

    unsigned long context_sw = current->nivcsw + current->nvcsw;
    printk("Number of context switches: %lu (voluntary: %lu, involuntary: %lu)\n", context_sw, current->nvcsw, current->nivcsw);
}

module_init(drv_init);
module_exit(drv_exit);