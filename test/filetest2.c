#include "syscall.h"
int f1,f2;
int result;
char buffer[13];
int
main()
{
	Create("test2.txt");
	Create("testdir");
	Create("testdir\\testdir2");
	Create("testdir\\testfile1");
	Create("testdir\\testfile2");
	Create("testdir\\testdir2\\lonenametestfile");
	Halt();
}
