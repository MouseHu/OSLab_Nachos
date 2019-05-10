#include "syscall.h"
int f1,f2;
int result;
char buffer[13];
int
main()
{
	Create("test2.txt");
	Create("hehehe");
	Create("hehehe\\hehe.txt");
	Halt();
}
