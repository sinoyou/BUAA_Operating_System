## BUAA OS 2019

*Individual Courseword of "Operating System Project" in Spring Semester*

### General Information

**Author**: Zinuo You

**Grade**: 94/100

**Reference**: MIT JOS (BUAA OS simplified the architecture from x86 to MIPS, for undergraduate education)

**Guide Book**: 
  - [BUAA OS Guide Book (2019 Version, Chinese)](assets/guide-book.pdf)
  - [See MIPS Run Linux (English)](assets/See-Mips-Run-Linux.pdf)



### Lab Introduction

#### Lab 0 - Warm-Up

- **Target**: Master the basic Linux commands, common tools, and Git usage in a command-line environment. 

#### Lab 1 - Kernel And Boot

- **Target**: 
  - Understand OS booting process and Gxemul MIPS simulator. 
  - Implement ELF file parsing and loading. 
  - Understand memory layout and load OS kernel to the proper address. 

#### Lab 2 - Memory Management 

- **Target**: 
  - Understand memory accessing through MMU and TLB, and memory mapping mechanism.
  - Implement physical memory management with queues and linked lists. 
  - Implement paged virtual memory management. 
- **Report:** [lab2-report(Chinese)](assets/lab2-report.pdf)

#### Lab 3 - Process and Interrupts

- **Target:** 
  - Understand "process control block". 
  - Implement process management to run an executable binary file. 
  - Implement process schduling on CPU timer interrupts. 
- **Report:** [lab3-report(Chinese)](assets/lab3-report.pdf)

#### Lab 4 - System Call and Fork

- **Target:**
  - Understand the function and the process of system call. 
  - Implemnet basic system calls of memory management and yield. 
  - Implement IPC process communication through system calls. 
  - Implement process Fork with "Copy On Write" and "Page Fault Handling".
- **Report**: [lab4-report(Chinese)](assets/lab4-report.pdf)

#### Lab 5 - File System

- **Target**:
  - Understand file system mechanism and microkernel design. 
  - Implement IDE external storage disk driver.
  - Implement the file system structures on Disks(blocks management with Bitmap) and on OS(file control blocks with singly indirect pointers). 
  - Implement the interfaces of the file system for interaction between user process and file system server. 
- **Report**: 
  - [lab5-report(Chinese)](assets/lab5-report.pdf)

#### Lab 6 - Pipe and Shell (Extra)

- **Target**:
  - Implement anonymous pipes communication under process competition environment. 
  - Understand the mechanism of Shell process with pipes. 
- **Report:**[lab6-report(Chinese)](assets/lab6-report.pdf)



#### Optional Challenge - Thread and Semaphore

- Implement a simplified POSIX Thread mechanism, which supports full sharing of user space and mutual stack access between threads with only a small amount of necessary private space. 
- Implement unnamed POSIX Semaphore for the synchronization and the communitcation in a process. 
- **Slide:**[lab-challenge-POXIS(Chinese)](assets/lab-challenge-POSIX.pdf)
