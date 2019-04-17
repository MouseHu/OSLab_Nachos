#include "syscall.h"

int main(){
int A[2][2];
int i,j;
for(i =0;i<2;i++){
	for(j =0;j<2;j++){
		A[j][i]=j;
	}	
	//Yield();
}
Exit(0);
return 0;
}
