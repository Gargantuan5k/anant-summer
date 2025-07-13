#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>

#define BUFFER_FILE "buffer.dat"
#define SLOT_SIZE 1024         // 1024 bytes
#define SLOT_COUNT 4           // 4x = 4096 bytes
#define HEADER_SIZE SLOT_COUNT // 1 byte header for each slot
#define BUFFER_SIZE (HEADER_SIZE + SLOT_COUNT * SLOT_SIZE)
#define SLOT_OFFSET(i) (HEADER_SIZE + (i) * SLOT_SIZE) // calculate byte offset where data for ith slot begins

void init_buf()
{
    int fd = open(BUFFER_FILE, O_RDWR | O_CREAT | O_TRUNC, 0666); // all users have r,w perms
    if (fd < 0)
    {
        perror("file open failed");
        exit(1);
    }

    char zeros[BUFFER_SIZE] = {0}; // all statuses, slots zeroed
    write(fd, zeros, BUFFER_SIZE);
    close(fd);
}

// Allows for a process to have exclusive control over the buffer file
void lock_file(int fd, short type)
{
    struct flock lock;
    lock.l_type = type; // F_RDLCK or F_WRLCK or F_UNLCK
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0; // lock the whole file

    while (fcntl(fd, F_SETLKW, &lock) == -1)
        ;
}

void log_slot_statuses(int fd, const char *label) {
    // prepare an array {x, x, x, x} containing statuses of each slot
    char statuses[SLOT_COUNT];
    lseek(fd, 0, SEEK_SET);
    read(fd, statuses, SLOT_COUNT);

    // Open log file in append mode
    FILE *log_fp = fopen("log.txt", "a");
    if (!log_fp) {
        perror("log.txt open failed");
        return;
    }

    // Get current time
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", t);

    // Write slot status with label and timestamp
    fprintf(log_fp, "[%s] %s - Slot statuses: ", time_str, label);
    for (int i = 0; i < SLOT_COUNT; i++) {
        fprintf(log_fp, "%d ", statuses[i]);
    }
    fprintf(log_fp, "\n");

    fclose(log_fp);
}


void producer(int log_switch) // log_switch flag: if we want to log slot statuses in each cycle
{
    int fd = open(BUFFER_FILE, O_RDWR);
    if (fd < 0)
    {
        perror("producer failed to open file");
        exit(1);
    }

    int slot = 0;
    int count = 0;

    while (1)
    {
        lock_file(fd, F_WRLCK); // exclusive read/write access to fd

        if (log_switch)
            log_slot_statuses(fd, "PRODUCER");

        char status;
        read(fd, &status, 1); // read the status byte

        if (status == 1) // slot is full
        {
            slot = (slot + 1) % SLOT_COUNT; // can have values 0 1 2 3 only
            lock_file(fd, F_UNLCK); // release the file
            usleep(500000); // wait and retry
            continue;
        }

        // Slot empry -> mark as full first
        lseek(fd, slot, SEEK_SET);
        status = 1;
        write(fd, &status, 1); 


        // create a message & write it to slot
        char msg[SLOT_SIZE];
        snprintf(msg, SLOT_SIZE, "Message %d from producer at %ld\n", count++, time(NULL));
        lseek(fd, SLOT_OFFSET(slot), SEEK_SET);
        write(fd, msg, SLOT_SIZE);

        printf("Producer wrote: %s", msg);

        slot = (slot + 1) % SLOT_COUNT;

        lock_file(fd, F_UNLCK);
        sleep(1); 
    }

    close(fd);
}

void consumer(int log_switch)
{
    int fd = open(BUFFER_FILE, O_RDWR);
    if (fd < 0)
    {
        perror("consumer failed to open buffer file");
        exit(1);
    }

    int slot = 0;

    while (1)
    {
        lock_file(fd, F_WRLCK); // exclusive access to consumer

        if (log_switch)
            log_slot_statuses(fd, "CONSUMER");

        // read the status bbyte
        lseek(fd, slot, SEEK_SET);
        char status;
        read (fd, &status, 1);

        if (status == 0)
        {
            slot = (slot + 1) % SLOT_COUNT;
            lock_file(fd, F_UNLCK);
            usleep(300000); // shorter wait time since producer is faster
            continue;
        }

        // read a message
        char msg[SLOT_SIZE + 1];
        lseek(fd, SLOT_OFFSET(slot), SEEK_SET);
        read(fd, msg, SLOT_SIZE);
        msg[SLOT_SIZE] = '\0'; // terminate

        printf("consumer read: %s", msg);

        // mark status of slot as empty
        lseek(fd, slot, SEEK_SET);
        status = 0;
        write(fd, &status, 1);

        slot = (slot + 1) % SLOT_COUNT;

        lock_file(fd, F_UNLCK); // release
        sleep(2); // simulating a slower consumer
    }

    close(fd);
}

int main(int argc, char const *argv[])
{
    // decide whether to log slot statuses
    int log_switch;
    if (argc == 1)
        log_switch = 0;
    else
        log_switch = atoi(argv[1]);

    init_buf();

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork failed");
        exit(1);
    }
    else if (pid == 0) // child - assumed consumer
    {
        consumer(log_switch);
    }
    else // parent - assumed producer
    {
        producer(log_switch);
    }

    return 0;
}
