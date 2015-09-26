#include "syscall.h"

int main()
{
   if (system_Fork() == 0)
   {
      system_PrintString("Hello, test string ");
      system_PrintInt(system_GetPID());
      system_PrintChar(' ');
      system_PrintInt(system_GetPPID());
      system_PrintChar('\n');
   }

   else
   {
      system_PrintString("Hello, parent's test string ");
      system_PrintInt(system_GetPID());
      system_PrintChar('\n');
   }
   return 0;
}
