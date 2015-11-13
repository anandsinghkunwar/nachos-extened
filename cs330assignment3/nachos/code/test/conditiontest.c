#include "synchop.h"
int
main()
{
   int *array = (int *)system_ShmAllocate(sizeof(int));
   int semid = system_SemGet(19);
   int i, x;
   int j = 1;
   system_SemCtl(semid, SYNCH_SET, &j);
   int cond = system_CondGet(19);
   for (i = 0; i < 5; i++)
   {
      x = system_Fork();
      if (x == 0)
      {
         system_SemOp(semid, -1);
         system_CondOp(cond, COND_OP_WAIT, semid);
         system_Exit(0);
      }
   }
   if (x != 0)
      system_CondOp(cond, COND_OP_BROADCAST, semid);

   system_CondRemove(cond);
   system_SemCtl(semid, SYNCH_REMOVE, 0);
   return 0;
}
