#include "syscall.h"

int
main()
{
    system_PrintInt(system_GetTime());
    system_PrintChar('\n');
    system_Sleep(600);
    system_PrintInt(system_GetTime());

    return 0;
}
