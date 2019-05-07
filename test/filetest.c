#include "syscall.h"
int f1,f2;
int result;
char buffer[13];
int
main()
{
	Create("test2.txt");
	f1 = Open("test1.txt");
	f2 = Open("test2.txt");	
	Read(buffer,13,f1);
	Write(buffer,13,f2);
	Close(f1);
	Close(f2);	
	Halt();
}
