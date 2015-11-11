#define KEY1 13
#define KEY2 17
#include "synchop.h"
int
main()
{
   int *count = (int*)system_ShmAllocate(sizeof(int));
   count[0] = 5;
   int s1, s2;
   s1 = system_SemGet(KEY1);
   s2 = system_SemGet(KEY2);
   int i = 0;
   system_SemCtl(s1, SYNCH_SET, &i);
   i = 1;
   system_SemCtl(s2, SYNCH_SET, &i);
   int x = system_Fork();
   if (x == 0)
   {
      system_SemOp(s2, -1);
      if (count[0] == 5) count[0] -= 2;
      else count[0] += 2;
      system_SemOp(s1, 1);
   }
   else
   {
      system_SemOp(s1, -1);
      if (count[0] == 5) count[0]--;
      else count[0]++;
      system_SemOp(s1, 1);
      system_Join(x);
      system_PrintString("Final value of count: ");
      system_PrintInt(count[0]);
      system_PrintChar('\n');
   }
   return 0;
}
