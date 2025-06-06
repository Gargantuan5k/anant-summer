# Task 1: Context switching

> A basic driver that, upon getting loaded, prints the PID of the parent process that loaded it, and upon getting removed, prints arbitrary messages and then the number of context switches that occurred for the current process.

> See Notes section for BONUS and some brief explanations

## Building and running
**Ensure you have linux kernel headers installed.**

Then in the directory where `pid_csw.c` and `Makefile` are located, run

```bash
sudo make
```

**To insert the module**
```bash
sudo insmod pid_csw.ko
```

**To remove the module**
```bash
sudo rmmod pid_csw
```

**To access the kernel log buffer to read output**
```bash
dmesg
```

## Notes

### Voluntary and involuntary context switches
- **Voluntary context switches** occur when a process explicitly yields the CPU (ex. by sleeping); their count is accessed using `current->nvcsw`.
- **Involuntary context switches** occur when the scheduler preempts a process because its time slice expired, or a higher-priority process needs the CPU; their count is accessed using `current->nivcsw`.

### Need for the `msleep()` calls in the for loop

If the loop is run without any `msleep()` calls, the number of context switches comes out to be ~0 or ~1.

This is likely because the loop runs quickly without yielding the CPU at all. If the process' time slice doesn't expire and no higher priority process needs the CPU, the number of context switches may remain zero.

`msleep()` causes the current process to *explicitly sleep*, causing a **voluntary context switch**.

### BONUS - Context switches before the parent PID was printed
> Does the count of context switches include those that occurred before the parent PID was printed?

**Short Answer: YES**

As seen from the kernel log messages, `current` refers to the `task_struct` of the current process, which is the module loader process (`insmod`).

We found the number of context switches by accessing the `nivcsw` and `nvcsw` fields of `current`. Since the current process is `insmod`, this count would include all context switches that occurred after the current task started, until the count was accessed.

When we query the number of context switches in the exit function, the count includes:
- Any switches that occurred before the module was inserted (**because `insmod` performs a bunch of tasks before it calls the `module_init` function**)
- Any switches during or after the init function
- Any that happened in the exit function

