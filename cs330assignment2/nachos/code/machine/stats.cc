// stats.h 
//	Routines for managing statistics about Nachos performance.
//
// DO NOT CHANGE -- these stats are maintained by the machine emulation.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "utility.h"
#include "stats.h"

//----------------------------------------------------------------------
// Statistics::Statistics
// 	Initialize performance metrics to zero, at system startup.
//----------------------------------------------------------------------

Statistics::Statistics()
{
    totalTicks = idleTicks = systemTicks = userTicks = 0;
    numDiskReads = numDiskWrites = 0;
    numConsoleCharsRead = numConsoleCharsWritten = 0;
    numPageFaults = numPacketsSent = numPacketsRecvd = 0;
    startTime = endTime = 0;
    minBurst = maxBurst = totalBurst = nonZeroBursts = 0;
    avgWaitTime = minCompletionTime = maxCompletionTime = 0;
    avgCompletionTime = completionTimeVariance = 0;
}

//----------------------------------------------------------------------
// Statistics::Print
// 	Print performance metrics, when we've finished everything
//	at system shutdown.
//----------------------------------------------------------------------

void
Statistics::Print()
{
    printf("Ticks: total %d, idle %d, system %d, user %d\n", totalTicks, 
	idleTicks, systemTicks, userTicks);
    printf("Disk I/O: reads %d, writes %d\n", numDiskReads, numDiskWrites);
    printf("Console I/O: reads %d, writes %d\n", numConsoleCharsRead, 
	numConsoleCharsWritten);
    printf("Paging: faults %d\n", numPageFaults);
    printf("Network I/O: packets received %d, sent %d\n", numPacketsRecvd, 
	numPacketsSent);
    printf("Total CPU busy time: %d\n", totalBurst);
    printf("Total execution time: %d, CPU Utilization: %f\n", endTime - startTime,
   (totalBurst*1.0)/(endTime - startTime));
    printf("CPU burst lengths: minimum %d, maximum %d, average %f\n", minBurst,
   maxBurst, totalBurst*1.0/nonZeroBursts);
    printf("Number of non zero bursts: %d\n", nonZeroBursts);
    printf("Average waiting time in the ready queue: %d\n", avgWaitTime);
    printf("Thread Completion time: minimum %d, maximum %d, average %d, variance %d\n",
   minCompletionTime, maxCompletionTime, avgCompletionTime, completionTimeVariance);
}
