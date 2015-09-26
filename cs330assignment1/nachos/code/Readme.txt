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
