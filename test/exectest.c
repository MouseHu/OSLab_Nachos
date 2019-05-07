#include "syscall.h"

int
main()
{
	int id;
	id = Exec("../test/halt");
	Join(id);
	Exit(0);
}
