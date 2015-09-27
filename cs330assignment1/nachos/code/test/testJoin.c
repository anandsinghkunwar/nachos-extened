#include "syscall.h"

int
main(){
	int x;
	x = system_Fork();
	if(x == 0){
		system_PrintString("In the child ");
		system_Sleep(500);
		system_Exit(15);
	}
	else{
		system_PrintString("In the parent ");
		system_PrintInt(system_Join(x));
	}
	return 0;
}
