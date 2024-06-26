# OS Assigment: Multi-threading library

## Problem
Implement 1-1 user level threading library.
```
What do we mean by this?
- 1-1: one user thread will map to one kernel thread
- user level: we will give user level functions to implement threads
```
> Implement userland threads on Linux systems, using 1-1 thread mapping.

### Threading basics

- A thread is a control flow with separate PC, registers and stack.
- Threads mostly share DS(thus static and globals), files and code area.
```
User-level threads:
- Library will provide the threading functions (creation, deletion, etc)
- Library will schedule the threads
```
The following functions should be implemented:

- thread_create(); // create a thread
- thread_join() ; // parent waits for thread
- thread_exit(); // thread exit, thread exits

> NOTE: Study the clone() system call on Linux thoroughly before you start.
```
The parameters for the three functions above should be defined by you.
Essentially the call to thread_create() involves a call to clone() with proper 
parameters, the call to thread_join() involves a call to some variant of wait()
with proper parameters. Think about thread_exit(). Of course, you need to take 
care of the list of processes also. You will also need to write a testing code,
considering all possibilities. 
```
Specifically, provide these files: thread.h (with all declarations), and thread.c (code of functions), and a testing code test.c

