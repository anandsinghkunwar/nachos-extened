CS330 Assignment 1 - Group 19

------------------------------------------------------------------------------------
syscall_GetReg
------------------------------------------------------------------------------------
Changes made in - exception.cc
Changes - We simply used the machine->ReadRegister method to read the number of
the argument register from register $4 and used the machine->WriteRegister method
to write the return value in register $2.

------------------------------------------------------------------------------------
syscall_GetPA
------------------------------------------------------------------------------------
Changes made in - exception.cc
Changes - We first compute the virtual page number of the virtual address and
then perform 3 checks. First - if it exceeds the page table size. Second - if its 
valid field entry in the page table is false. Third - if the physical page number
corresponding to it is larger than the number of physical pages. If any of the
checks evaluate to true, we return -1, otherwise we compute the physical address by
adding the offset (virtual address % page size) to the product of physical page 
number & page size and return it.

------------------------------------------------------------------------------------
syscall_GetPID
------------------------------------------------------------------------------------
Changes made in - thread.cc, thread.h, exception.cc
Changes - In the NachOSThread constructor method, we declared a static integer
variable called currentPID which keeps assigning PID to the thread and incrementing
itself. We added a getter method in the NachOSThread class to return the PID of the
thread, which is the return value of the syscall.

------------------------------------------------------------------------------------
syscall_GetPPID
------------------------------------------------------------------------------------
Changes made in - thread.cc, thread.h, exception.cc
Changes - We added a condition in the ThreadFork method to set the PPID of the 
forked thread to the calling thread's PID. We also added a getter method in the
NachOSThread class to return the PPID of the thread, which is the return value of
the syscall.

------------------------------------------------------------------------------------
syscall_Time
------------------------------------------------------------------------------------
Changes made in - exception.cc 
Changes - stats variable is a pointer to an object of Statistics class.
We got the total ticks from stats->totalTicks and returned this value from the 
system call.

------------------------------------------------------------------------------------
syscall_Yield
------------------------------------------------------------------------------------
Changes made in - exception.cc
Changes - We had to take help of the NachOSThread::YieldCPU() method. 
We called this function on the current thread using the pointer of our 
current thread(currentThread).

------------------------------------------------------------------------------------
syscall_Sleep
------------------------------------------------------------------------------------
Changes made in - exception.cc, system.h, system.cc
Changes - First we declared a global pointer to the List of sleeping threads with 
name "sleepingThreads" in file system.h by attaching extern in its declaration. 
We then initialised sleepThreads as a new empty List in the Initialize method of
system.cc. In exception.cc, we inserted the current thread with key as the sum of
total ticks of the system and the number of ticks (input in register $4) to this
sleepingThreads List. Then we called the PutThreadToSleep() method on the 
current thread and advanced the program counters. In system.cc 
TimerInterruptHandler() method, we edited it to remove all the threads which have
their key (totalTicks at which they have wake up) less than or equal to the 
total ticks of the system and to put them in the ready queue. 

------------------------------------------------------------------------------------
syscall_Exec
------------------------------------------------------------------------------------
Changes made in - exception.cc, addrspace.cc, addrspace.h
Changes - We added a static integer variable called totalAllocatedPages in the
addrspace class, which stores the number of physical pages which have been allocated
to all threads. This value also corresponds to the first unallocated physical page.
We allocated space to new threads in main memory starting from the first physical
page that is unallocated. We added a getStartAddress method which returns the start
address of the space according to the changes mentioned above. We increment the
totalAllocatedPages variable after allocating a space.
For the system call, we first read the filename of the executable to be loaded in a
string, and then merely mimic the StartProcess function. We use that name to load 
the executable's address space and attach it to the running thread.

------------------------------------------------------------------------------------
syscall_Exit
------------------------------------------------------------------------------------
Changes made in - exception.cc, system.h, system.cc, thread.cc, thread.h
Changes - We declared a global variable numThreads which contains the number
of processes running/sleeping/waiting in file system.h by attaching extern in its
declaration. We initialised this to 0 in Initialize method of system.cc. Whenever a
thread is created, we increment numThreads by one in the thread constructor. We 
decrease this variable by when NachOSThread::FinishThread() method is called.
In the thread class, we also include a pointer to the parent thread (if any), and
a list of alive children threads. We store another list, exitedChildProcesses,
which keeps the exit codes of the dead children threads, along with their PID. Now,
when system_Exit is called, we set the exit status as the return value, and then
use the numThreads variable to check if the exiting thread is the only thread in the
simulator. If so, the machine is simply halted. Otherwise, we find the exiting 
thread's parent and remove the exiting thread from the parent's aliveChildProcesses
list, and add the exit code to the exitedChildProcesses list. We check the parent's
waitPid variable, and if it matches with the PID of the exiting thread, we reset its
waitPid variable and wake it up. Then, we go to the list of the children of the 
exiting thread, and set their parent thread pointer to NULL and PPID to 0. Then, the
exiting thread calls FinishThread.
