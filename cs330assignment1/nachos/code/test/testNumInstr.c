#include "syscall.h"
// #define SIZE 100

int main ()
{
    int a;
    system_PrintString("I am printing: ");
    a = system_GetNumInstr();
    system_PrintInt(a);
    system_PrintChar('\n');
    return 0;
}
