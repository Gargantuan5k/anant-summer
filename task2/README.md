# Task 2: Producer-Consumer problem

> Implement a producer-consumer process relationship by forking, using a file as a circular buffer

## Build & Run

```sh
gcc prod_cons.c
./a.out <log_switch>
# log_switch flag: 0 or 1, decides whether to log slot statuses to a .txt file.
```

**In another terminal simultaneously, run:**
```sh
python3 inspect_buffer.py
```

## Notes

`init_buf()` initialises a buffer file of reqd specs, with zeroed data.

`lock_file()` allows a process to gain exclusive R/W over the buffer file.

#### `buffer.dat` buffer file:

4 "slots" of 1024B each, where producer will write the data, and 4 "status" bytes at the start.

The first 4 bytes contain the "status" of each "slot" in the file. `0` means the slot is EMPTY, or has already been READ by the consumer.
`1` means the slot is FULL, i.e. WRITTEN by the producer but NOT READ yet by consumer.

#### `producer()`:

1. Lock the file and log the statuses
2. Read a status byte.
    - if `1`: (SLOT FULL), skip this slot, go to the next.
    - if `0`: write a 1024B message and set status to `1`.
3. Unlock the file
4. Sleep for 1s to simulate a delay

#### `consumer()`:
1. Lock the file and log statuses
2. Read a status byte.
    - if `0`: (SLOT EMPTY), skip this slot, go to next
    - if `1`: read the message in this slot, set status to `0`.
3. Unlock the file
4. Sleep for 2s (longer than the producer; hence we guarantee that the consumer is SLOWER)


### `inspect_buffer.py` script:

A python script that reads the buffer.dat file separately and gives a live preview of slot statuses and slot contents. Acts as a live dashboard, to be run simultaneously with the main program.

### Unexpected behaviour - UPDATE: FIXED!

Producer process was not looping back to the start of the file to check the status bytes after the first full cycle :(

Fixed by adding an lseek() call.


#### Archive note

Here's the original "Unexpected behaviour" issue:

1. `producer()` writes only 4 messages (which are read by `consumer()`). After 4 messages, output stops and both processes seem to stall.
    - inspection revealed that for some reason, all status bytes are set to `1` after 4 messages. Unclear whether this happens after consumer is finished reading or somewhere in between.
    - producer and consumer APPEAR to be updating status bytes correctly??


2. Created a logging function to check status bytes in real time.
    - when logging function is called, everything works fine
    - more than 4 messages are written and read
    - tweaking speeds of consumer and producer (by changing sleep() times), does NOT break the program. wtf??


3. My theory is that this behaviour is due to some sort of timing issue or buffer flushing issue, that gets fixed when the log loop runs
    - (this is guesswork, i have no idea if this is actually what's happening)