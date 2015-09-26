syscall_Time
Changes made in - exception.cc 
Changes - stats variable is a pointer to an object of Statistics class. We got the total ticks from stats->totalTicks and returned this value from the system call.

syscall_Yield
Changes made in - exception.cc
Changes - We had to take help of the NachOSThread::YieldCPU() method. We called this function on the current thread using the pointer of our current thread(curentThread).