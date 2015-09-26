#include "syscall.h"

int main()
{
   int a = system_Fork();
   if (a == 0)
   {
      system_PrintString("Hellolong");
      system_PrintChar('\n');
   }

   else
   {
      system_PrintInt(a);
      system_PrintChar('\n');
   }
   
   system_PrintChar('X');
   return 0;
}
