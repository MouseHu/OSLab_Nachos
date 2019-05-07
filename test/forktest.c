#include "syscall.h"
int f1,f2;
char buffer[13];
char test; 
void fork_func(){
	test = '1';
	f1 = Open("test1.txt");
	Read(buffer,13,f1);
	Close(f1);
}
int
main()
{
	test = '0';
	Create("test_fork.txt");
	Fork(fork_func);
	Yield();
	f2 = Open("test_fork.txt");
	Write(buffer,13,f2);
	Write(&test,1,f2);
	Close(f2);	
	Exit(0);
}
