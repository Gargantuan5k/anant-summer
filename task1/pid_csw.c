#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/delay.h>
// #include <sched.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Siddharth Vivek");
MODULE_DESCRIPTION("A basic module that prints the parent process ID and the number of context switches");

static int __init drv_init(void)
{
    struct task_struct *ts = current;

    if (current->parent)
    {
        printk(KERN_INFO "loaded by parent with ID: %d\n", current->parent->pid);
    }
    else
    {
        printk(KERN_ERR "could not get parent PID\n");
        return 1;
    }

    return 0;
}

static void __exit drv_exit(void)
{
    struct task_struct *ts = current;
    int i;

    for (i = 0; i < 5; i++)
    {
        printk(KERN_INFO "random message %d\n", i);
        msleep(10);
    }

    // TODO context switching thingy
}

module_init(drv_init);
module_exit(drv_exit);