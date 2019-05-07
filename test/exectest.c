#include "syscall.h"

int
main()
{
	int id;
	id = Exec("halt");
	Join(id);
	Exit(0);
}
