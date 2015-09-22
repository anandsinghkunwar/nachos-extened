#include "syscall.h"

int main ()
{
	unsigned int val;
	system_PrintString("start");
	val = system_NumInstr();
	system_PrintString("end");
	return 0;
}
