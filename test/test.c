#include "syscall.h"

int main(){
int A[20][20];
int i,j;	
for(i =0;i<20;i++){
	for(j =0;j<20;j++){
		A[j][i]=j;
	}	
	//Yield();
}

Exit(0);
}
