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
#include "system.h"

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
    GetStats();
    printf("Average waiting time in the ready queue: %f\n", avgWaitTime);
    printf("Thread Completion time: minimum %d, maximum %d, average %f, variance %f\n",
   minCompletionTime, maxCompletionTime, avgCompletionTime, completionTimeVariance);
}

//----------------------------------------------------------------------
// Statistics::GetStats
//    Compute average waiting time, max, min, average completion time
// and variance of completion time.
//----------------------------------------------------------------------

void
Statistics::GetStats()
{
   int i, temp;
   for (i = 0; i < thread_index; i++)
      avgWaitTime += threadWaitTime[i];
   avgWaitTime = avgWaitTime/thread_index;

   for (i = 1; i < thread_index; i++) {
      temp = threadCompletionTime[i];
      if ((minCompletionTime == 0 || temp < minCompletionTime))
         minCompletionTime = temp;
      if ((maxCompletionTime == 0 || temp > maxCompletionTime))
         maxCompletionTime = temp;
      avgCompletionTime += temp;
   }
   avgCompletionTime = avgCompletionTime/(thread_index - 1);

   for (i = 1; i < thread_index; i++)
   {
      temp = threadCompletionTime[i];
      completionTimeVariance += (temp - avgCompletionTime)*(temp - avgCompletionTime);
   }
   completionTimeVariance = completionTimeVariance/(thread_index - 1);
}
