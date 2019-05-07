#include "syscall.h"
int f1,f2;
char buffer[13]; 
void fork_func(){
	f1 = Open("test1.txt");
	Read(buffer,13,f1);
	Close(f1);
	Exit(0);
}
int
main()
{
	buffer[0]='1';
	Create("test_fork.txt");
	Fork(fork_func);
	Yield();
	f2 = Open("test_fork.txt");
	Write(buffer,13,f2);
	Close(f2);	
	Halt();
}
