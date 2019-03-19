#include <philosopher.h>

Philosopher* philosophers[num_philosopher];
bool chops[num_philosopher];
Lock* table = new Lock("table");

void PhiloInit(){

    for(int i =0;i<num_philosopher;i++){
        philosophers[i]=new Philosopher(i);
        chops[i]=false;
    }
    for(int i =0;i<num_philosopher;i++){
        philosophers[i]->SetNeighbor(philosophers[(i-1+num_philosopher)%num_philosopher],philosophers[(i+1)%num_philosopher]);
    }
}

void takeChop(bool* chop){
    table->Acquire();
    *chop = true;
    table->Release();
}
bool check(bool* chop){
    table->Acquire();
    bool c = *chop;
    table->Release();
    return c;
}
void PrintTable(){
    for(int i =0;i<num_philosopher;i++){
        printf("%d ",int(chops[i]));
    }
    printf("\n");
    
}
Philosopher::Philosopher(int id){
    this->id= id;
    chopnum=0;
    eating = false;
    cond = new Condition("None");
    left_chop=false;
    right_chop = false;
}

Philosopher::~Philosopher(){}

void Philosopher::SetNeighbor(Philosopher* left,Philosopher* right){
    this->left_neighbor=left;
    this->right_neighbor=right;
}

void Philosopher::PickChop(){
    IntStatus old = interrupt->SetLevel(IntOff);
    printf("Philosopher %d picking chops.\n",id);
    
    if(chopnum==0){
        while(check(chops+(id-1+num_philosopher)%num_philosopher) ||check(chops+(id+num_philosopher)%num_philosopher) ){
            printf("philo%d wait: %d %d\n",id, *(chops+(id-1+num_philosopher)%num_philosopher),*(chops+(id+num_philosopher)%num_philosopher));
            cond->Wait(table);
            printf("philo%d wait: %d %d\n",id, *(chops+(id-1+num_philosopher)%num_philosopher),*(chops+(id+num_philosopher)%num_philosopher));
        }
        if(id%2){
            takeChop(chops+(id-1+num_philosopher)%num_philosopher);
            left_chop = true;
        }
        else{
            takeChop(chops+(id+num_philosopher)%num_philosopher);
            right_chop = true;
        }
    }
    else if(chopnum==1){
        if(left_chop){
            while(check(chops+(id+num_philosopher)%num_philosopher)){
                printf("philo%d wait: %d\n",id,*(chops+(id+num_philosopher)%num_philosopher));
                cond->Wait(table);
                //printf("philo%d wait: %d\n",id,*(chops+(id+num_philosopher)%num_philosopher));
            }
            takeChop(chops+(id+num_philosopher)%num_philosopher);
            right_chop=true;
        }
        else{
            ASSERT(right_chop)
            while(check(chops+(id-1+num_philosopher)%num_philosopher)){
                printf("philo%d wait: %d\n",id,*(chops+(id-1+num_philosopher)%num_philosopher));
                cond->Wait(table);
            }
            takeChop(chops+(id-1+num_philosopher)%num_philosopher);
            left_chop=true;
        }
    }
    printf("Philosopher %d picks a chop.\n",id);
    PrintTable();
    chopnum++;
    interrupt->SetLevel(old);
}

void Philosopher::Eat(){
    ASSERT(left_chop && right_chop);
    
    eating = true;
    printf("Philosopher %d eating.\n",id);
    PrintTable();
    int n = 20;
    for(int i =0;i<n;i++){
        IntStatus old = interrupt->SetLevel(IntOff);
        interrupt->SetLevel(old);
    }
    IntStatus old = interrupt->SetLevel(IntOff);
    eating = false;
    printf("Philosopher %d finished eating.\n",id);
    
    table->Acquire();
    chops[(id)%num_philosopher]=false;
    table->Release();
    right_neighbor->cond->Signal(table);
    chopnum--;
    right_chop=false;
    
    table->Acquire();
    chops[(id-1+num_philosopher)%num_philosopher]=false;
    table->Release();
    left_neighbor->cond->Signal(table);
    chopnum--;
    left_chop=false;

    PrintTable();
    interrupt->SetLevel(old);
}

void Philosopher::Think(){
    printf("Philosopher %d thinking.\n",id);
    PrintTable();
    int n = 8;
    for(int i =0;i<n;i++){
        IntStatus old = interrupt->SetLevel(IntOff);
        interrupt->SetLevel(old);
    }
    printf("Philosopher %d think for %d times.\n",id,n);
}

void Action(int p){
    for(int i =0;i<3;i++){
        Think(p);
        //currentThread->Yield();
        PickChop(p);
        //currentThread->Yield();
        PickChop(p);
        //currentThread->Yield();
        Eat(p);
        //currentThread->Yield();
    }
    printf("%s finishes.\n",currentThread->getName());
}