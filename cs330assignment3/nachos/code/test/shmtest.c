int
main()
{
   int *var = (int*)system_ShmAllocate(sizeof(int));
   int x = system_Fork();
   if (x == 0)
   {
      var[0] = 4;
      system_PrintString("Child saying value is: ");
      system_PrintInt(var[0]);
      system_PrintChar('\n');
   }
   else
   {
      var[0] = 1;
      system_PrintString("Parent saying value before join is: ");
      system_PrintInt(var[0]);
      system_PrintChar('\n');
      system_Join(x);
      system_PrintString("Parent saying value after join is: ");
      system_PrintInt(var[0]);
      system_PrintChar('\n');
   }
   return 0;
}
