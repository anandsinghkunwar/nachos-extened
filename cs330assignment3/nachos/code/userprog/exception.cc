// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "console.h"
#include "synch.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------
static Semaphore *readAvail;
static Semaphore *writeDone;
static void ReadAvail(int arg) { readAvail->V(); }
static void WriteDone(int arg) { writeDone->V(); }

extern void StartProcess (char*);

void
ForkStartFunction (int dummy)
{
   currentThread->Startup();
   machine->Run();
}

static void ConvertIntToHex (unsigned v, Console *console)
{
   unsigned x;
   if (v == 0) return;
   ConvertIntToHex (v/16, console);
   x = v % 16;
   if (x < 10) {
      writeDone->P() ;
      console->PutChar('0'+x);
   }
   else {
      writeDone->P() ;
      console->PutChar('a'+x-10);
   }
}

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
    int memval, vaddr, printval, tempval, exp;
    unsigned printvalus;	// Used for printing in hex
    if (!initializedConsoleSemaphores) {
       readAvail = new Semaphore("read avail", 0);
       writeDone = new Semaphore("write done", 1);
       initializedConsoleSemaphores = true;
    }
    Console *console = new Console(NULL, NULL, ReadAvail, WriteDone, 0);
    int exitcode;		// Used in syscall_Exit
    unsigned i;
    char buffer[1024];		// Used in syscall_Exec
    int waitpid;		// Used in syscall_Join
    int whichChild;		// Used in syscall_Join
    NachOSThread *child;		// Used by syscall_Fork
    unsigned sleeptime;		// Used by syscall_Sleep
    int semKey;            // Semaphore key used by syscall_SemGet
    Semaphore *newSemaphore; // Used by syscall_SemGet
    int semID, adjustValue;  // Used by syscall_SemOp and syscall_SemCtl
    int value;              // Used by syscall_SemCtl
    unsigned command;        // Used by syscall_SemCtl
    unsigned size, numSharedPages, currentNumPages, startAddress;   // Used by syscall_ShmAllocate
    TranslationEntry *currentPageTable, *newPageTable;  // Used by syscall_ShmAllocate
    int condKey, condID, op;     // Used by syscall_CondGet, syscall_CondOp, syscall_CondRemove
    Condition *newCondition; // Used by syscall_CondGet

    if ((which == SyscallException) && (type == syscall_Halt)) {
	DEBUG('a', "Shutdown, initiated by user program.\n");
   	interrupt->Halt();
    }
    else if ((which == SyscallException) && (type == syscall_Exit)) {
       exitcode = machine->ReadRegister(4);
       printf("[pid %d]: Exit called. Code: %d\n", currentThread->GetPID(), exitcode);
       // We do not wait for the children to finish.
       // The children will continue to run.
       // We will worry about this when and if we implement signals.
       exitThreadArray[currentThread->GetPID()] = true;

       // Find out if all threads have called exit
       for (i=0; i<thread_index; i++) {
          if (!exitThreadArray[i]) break;
       }
       currentThread->Exit(i==thread_index, exitcode);
    }
    else if ((which == SyscallException) && (type == syscall_Exec)) {
       // Copy the executable name into kernel space
       vaddr = machine->ReadRegister(4);
       while(!machine->ReadMem(vaddr, 1, &memval));
       i = 0;
       while ((*(char*)&memval) != '\0') {
          buffer[i] = (*(char*)&memval);
          i++;
          vaddr++;
          while(!machine->ReadMem(vaddr, 1, &memval));
       }
       buffer[i] = (*(char*)&memval);
       currentThread->space->FreePhysPages();
       delete currentThread->space;
       StartProcess(buffer);
    }
    else if ((which == SyscallException) && (type == syscall_Join)) {
       waitpid = machine->ReadRegister(4);
       // Check if this is my child. If not, return -1.
       whichChild = currentThread->CheckIfChild (waitpid);
       if (whichChild == -1) {
          printf("[pid %d] Cannot join with non-existent child [pid %d].\n", currentThread->GetPID(), waitpid);
          machine->WriteRegister(2, -1);
          // Advance program counters.
          machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
          machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
          machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
       }
       else {
          exitcode = currentThread->JoinWithChild (whichChild);
          machine->WriteRegister(2, exitcode);
          // Advance program counters.
          machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
          machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
          machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
       }
    }
    else if ((which == SyscallException) && (type == syscall_Fork)) {
       // Advance program counters.
       machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
       machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
       machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
       
       child = new NachOSThread("Forked thread", GET_NICE_FROM_PARENT);
       child->space = new AddrSpace (currentThread->space);  // Duplicates the address space
       child->SaveUserState ();		     		      // Duplicate the register set
       child->ResetReturnValue ();			     // Sets the return register to zero
       child->ThreadStackAllocate (ForkStartFunction, 0);	// Make it ready for a later context switch
       child->Schedule ();
       machine->WriteRegister(2, child->GetPID());		// Return value for parent
    }
    else if ((which == SyscallException) && (type == syscall_Yield)) {
       currentThread->YieldCPU();
       // Advance program counters.
       machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
       machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
       machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
    }
    else if ((which == SyscallException) && (type == syscall_PrintInt)) {
       printval = machine->ReadRegister(4);
       if (printval == 0) {
          writeDone->P() ;
          console->PutChar('0');
       }
       else {
          if (printval < 0) {
             writeDone->P() ;
             console->PutChar('-');
             printval = -printval;
          }
          tempval = printval;
          exp=1;
          while (tempval != 0) {
             tempval = tempval/10;
             exp = exp*10;
          }
          exp = exp/10;
          while (exp > 0) {
             writeDone->P() ;
             console->PutChar('0'+(printval/exp));
             printval = printval % exp;
             exp = exp/10;
          }
       }
       // Advance program counters.
       machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
       machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
       machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
    }
    else if ((which == SyscallException) && (type == syscall_PrintChar)) {
        writeDone->P() ;        // wait for previous write to finish
        console->PutChar(machine->ReadRegister(4));   // echo it!
       // Advance program counters.
       machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
       machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
       machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
    }
    else if ((which == SyscallException) && (type == syscall_PrintString)) {
       vaddr = machine->ReadRegister(4);
       while(!machine->ReadMem(vaddr, 1, &memval));
       while ((*(char*)&memval) != '\0') {
          writeDone->P() ;
          console->PutChar(*(char*)&memval);
          vaddr++;
          while(!machine->ReadMem(vaddr, 1, &memval));
       }
       // Advance program counters.
       machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
       machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
       machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
    }
    else if ((which == SyscallException) && (type == syscall_GetReg)) {
       machine->WriteRegister(2, machine->ReadRegister(machine->ReadRegister(4))); // Return value
       // Advance program counters.
       machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
       machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
       machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
    }
    else if ((which == SyscallException) && (type == syscall_GetPA)) {
       vaddr = machine->ReadRegister(4);
       machine->WriteRegister(2, machine->GetPA(vaddr));  // Return value
       // Advance program counters.
       machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
       machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
       machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
    }
    else if ((which == SyscallException) && (type == syscall_GetPID)) {
       machine->WriteRegister(2, currentThread->GetPID());
       // Advance program counters.
       machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
       machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
       machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
    }
    else if ((which == SyscallException) && (type == syscall_GetPPID)) {
       machine->WriteRegister(2, currentThread->GetPPID());
       // Advance program counters.
       machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
       machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
       machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
    }
    else if ((which == SyscallException) && (type == syscall_Sleep)) {
       sleeptime = machine->ReadRegister(4);
       if (sleeptime == 0) {
          // emulate a yield
          currentThread->YieldCPU();
       }
       else {
          currentThread->SortedInsertInWaitQueue (sleeptime+stats->totalTicks);
       }
       // Advance program counters.
       machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
       machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
       machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
    }
    else if ((which == SyscallException) && (type == syscall_Time)) {
       machine->WriteRegister(2, stats->totalTicks);
       // Advance program counters.
       machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
       machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
       machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
    }
    else if ((which == SyscallException) && (type == syscall_PrintIntHex)) {
       printvalus = (unsigned)machine->ReadRegister(4);
       writeDone->P() ;
       console->PutChar('0');
       writeDone->P() ;
       console->PutChar('x');
       if (printvalus == 0) {
          writeDone->P() ;
          console->PutChar('0');
       }
       else {
          ConvertIntToHex (printvalus, console);
       }
       // Advance program counters.
       machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
       machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
       machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
    }
    else if ((which == SyscallException) && (type == syscall_NumInstr)) {
       machine->WriteRegister(2, currentThread->GetInstructionCount());
       // Advance program counters.
       machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
       machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
       machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
    }
    else if ((which == SyscallException) && (type == syscall_ShmAllocate)) {
       size = machine->ReadRegister(4);
       numSharedPages = divRoundUp(size, PageSize);
       size = numSharedPages * PageSize;
       // Check number of physical pages
       ASSERT(numSharedPages+numPagesAllocated <= NumPhysPages);
       // Copy current page table into new page table
       currentNumPages = currentThread->space->GetNumPages();
       currentPageTable = currentThread->space->GetPageTable();
       newPageTable = new TranslationEntry[currentNumPages+numSharedPages];
       for (i = 0; i < currentNumPages; i++) {
          newPageTable[i] = currentPageTable[i];
       }
       // Allocate new physical pages for the shared region and set the translation entries
       for (i = currentNumPages; i < currentNumPages+numSharedPages; i++) {
          newPageTable[i].virtualPage = i;
          newPageTable[i].physicalPage = NextAvailPhysPage();
          bzero(&machine->mainMemory[newPageTable[i].physicalPage * PageSize], PageSize);
          physPageStatus[newPageTable[i].physicalPage] = TRUE;
          newPageTable[i].valid = TRUE;
          newPageTable[i].use = FALSE;
          newPageTable[i].dirty = FALSE;
          newPageTable[i].shared = TRUE;
          newPageTable[i].readOnly = FALSE;
          stats->numPageFaults++;
          numPagesAllocated++;
       }
       startAddress = currentNumPages*PageSize;
       currentThread->space->setPageTable(newPageTable);
       currentThread->space->addNumSharedPages(numSharedPages);
       // Delete the old page table and set the machine pagetable
       delete currentPageTable;
       machine->pageTable = newPageTable;
       machine->pageTableSize = currentNumPages+numSharedPages;

       machine->WriteRegister(2, startAddress);
       // Advance program counters.
       machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
       machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
       machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
    }
    // System calls for synchronization primitives
    else if ((which == SyscallException) && (type == syscall_SemGet)) {
       semKey = machine->ReadRegister(4);
       for (i = 0; i < semaphoreIndex; i++) {
          if ((semaphoreArray[i] != NULL) && (semaphoreKeys[i] == semKey)) {
             machine->WriteRegister(2, i);
             break;
          }
       }
       if (i == semaphoreIndex) {    // If the above loop did not find any semaphore with key = semKey
          newSemaphore = new Semaphore("Created Semaphore", 0);
          semaphoreKeys[newSemaphore->getID()] = semKey;
          machine->WriteRegister(2, newSemaphore->getID());
       }
       // Advance program counters.
       machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
       machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
       machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
    }
    else if ((which == SyscallException) && (type == syscall_SemOp)) {
       semID = machine->ReadRegister(4);
       adjustValue = machine->ReadRegister(5);
       if (semaphoreArray[semID] != NULL) {
          if (adjustValue == -1) {   // Wait operation
             semaphoreArray[semID]->P();
             machine->WriteRegister(2, 0);
          }
          else if (adjustValue == 1) {   // Signal operation
             semaphoreArray[semID]->V();
             machine->WriteRegister(2, 0);
          }
          else                          // Invalid operation
             machine->WriteRegister(2, -1);
       }
       else                            // No semaphore with the given ID exists
          machine->WriteRegister(2, -1);
       // Advance program counters.
       machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
       machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
       machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
    }
    else if ((which == SyscallException) && (type == syscall_SemCtl)) {
       semID = machine->ReadRegister(4);
       command = machine->ReadRegister(5);
       vaddr = machine->ReadRegister(6);
       if (command == SYNCH_REMOVE) {
          delete semaphoreArray[semID];
          semaphoreArray[semID] = NULL;
          machine->WriteRegister(2, 0);
       }
       else if (command == SYNCH_GET) {
          if (semaphoreArray[semID] != NULL) {
             value = semaphoreArray[semID]->getValue();
             while(!machine->WriteMem(vaddr, 4, value));
             machine->WriteRegister(2, 0);
          }
          else
             machine->WriteRegister(2, -1);
       }
       else if (command == SYNCH_SET) {
          if (semaphoreArray[semID] != NULL) {
             while(!machine->ReadMem(vaddr, 4, &value));
             semaphoreArray[semID]->setValue(value);
             machine->WriteRegister(2, 0);
          }
          else
             machine->WriteRegister(2, -1);
       }
       else
          machine->WriteRegister(2, -1);
       // Advance program counters.
       machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
       machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
       machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
    }
    else if ((which == SyscallException) && (type == syscall_CondGet)) {
       condKey = machine->ReadRegister(4);
       for (i = 0; i < conditionIndex; i++) {
          if ((conditionArray[i] != NULL) && (conditionKeys[i] == condKey)) {
             machine->WriteRegister(2, i);
             break;
          }
       }
       if (i == conditionIndex) {    // If the above loop did not find any cv with key = condKey
          newCondition = new Condition("Created Condition");
          conditionKeys[newCondition->getID()] = condKey;
          machine->WriteRegister(2, newCondition->getID());
       }
       // Advance program counters.
       machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
       machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
       machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
    }
    else if ((which == SyscallException) && (type == syscall_CondOp)) {
       condID = machine->ReadRegister(4);
       op = machine->ReadRegister(5);
       semID = machine->ReadRegister(6);

       if (conditionArray[condID] != NULL) {
          if (op == COND_OP_WAIT) {
             if (semaphoreArray[semID] != NULL) {
                conditionArray[condID]->Wait(semaphoreArray[semID]);
                machine->WriteRegister(2, 0);
             }
             else
                machine->WriteRegister(2, -1);
          }
          else if (op == COND_OP_SIGNAL) {
             conditionArray[condID]->Signal();
             machine->WriteRegister(2, 0);
          }
          else if (op == COND_OP_BROADCAST) {
             conditionArray[condID]->Broadcast();
             machine->WriteRegister(2, 0);
          }
          else
             machine->WriteRegister(2, -1);
       }
       else
          machine->WriteRegister(2, -1);

       // Advance program counters.
       machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
       machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
       machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
    }
    else if ((which == SyscallException) && (type == syscall_CondRemove)) {
       condID = machine->ReadRegister(4);
       delete conditionArray[condID];
       conditionArray[condID] = NULL;
       machine->WriteRegister(2, 0);
       // Advance program counters.
       machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
       machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
       machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
    }
    else if (which == PageFaultException) {
       stats->numPageFaults++;
       currentThread->SortedInsertInWaitQueue (1000+stats->totalTicks);   //Need to sleep for 1000 ticks to simulate latency
    }
    else {
	printf("Unexpected user mode exception %d %d\n", which, type);
	ASSERT(FALSE);
    }
}
