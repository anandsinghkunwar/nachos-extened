#include "syscall.h"

int main()
{
   int b,a = system_Fork();
   if (a!=0)
   {
      b = system_Fork();
      if(b==0) {
         system_Sleep(1000);
         system_PrintString("BBBBBBBB");
         system_PrintChar('\n');
      }
      else {
         system_Join(a);
         system_PrintString("CCCCCCCC");
         system_PrintChar('\n');
      }
   }
   if(a==0)
   {
      system_PrintString("AAAAAAAA");
      system_PrintChar('\n');
   }
   
   
   return 0;
}
