CS330 Assignment 1 - Group 19

------------------------------------------------------------------------------------
syscall_GetReg
------------------------------------------------------------------------------------
Changes made in - exception.cc
Changes - We simply used the machine->ReadRegister method to read the number of
the argument register from register $4 and used the machine->WriteRegister method
to write the return value in register $2.

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
syscall_Exit
------------------------------------------------------------------------------------
Changes made in - exception.cc, system.h, system.cc, thread.cc
Changes - First we declared a global variable numThreads which contains the number
of processes running/sleeping/waiting in file system.h by attaching extern in its
declaration. We initialised this to 0 in Initialize method of system.cc. Whenever a
thread is created, we increment variable numThreads by one in the constructor. We 
decrease this variable by when NachOSThread::FinishThread() method is called and 
also when exit syscall executes in exception.cc. When this variable is zero we exit
our program. 