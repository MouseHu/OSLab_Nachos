#include <barrier.h>

Barrier* barrier=new Barrier(barrier_count);
int count=0;

Barrier::Barrier(int c){
    count = c;
    count_lock = new Lock("barrier lock");
    cond = new Condition("barrier condition");
}

void Barrier::Synch(){
    IntStatus old = interrupt->SetLevel(IntOff);
    count_lock->Acquire();
    count--;
    if(count==0){
        printf("count: %d, Broadcast.\n",count);
        cond->Broadcast(count_lock);
    }
    else{
        printf("count: %d, Wait.\n",count);
        cond->Wait(count_lock);
    }
    count_lock->Release();
    interrupt->SetLevel(old);
}

void Add(int n){
    for(int i =0;i<n;i++){
        count+=1;
        IntStatus old = interrupt->SetLevel(IntOff);
        interrupt->SetLevel(old);
    }
    barrier->Synch();
    printf("thread:%s finishes,total sum:%d\n",currentThread->getName(),count);
}