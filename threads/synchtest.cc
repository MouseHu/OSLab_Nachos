//add by huhao used to test synchronize threads
#include "copyright.h"
#include "system.h"
#include "synchlist.h"
#include "wrlock.h"
#include "producerconsumer.h"
#include "philosopher.h"
#include "barrier.h"
#include <string>
#include<sstream>
// prolist using lock 

extern int testnum;
extern int list_count;
extern const int max_product;
extern SynchList* proList;

//philosopher
extern Philosopher* philosophers[num_philosopher];
extern Lock* table;

extern Barrier* barrier;
extern int count;
// Producer Consumer test for semaphore

extern int file;
extern WRlock* wrlock;

void
SynchTest1()
{
    DEBUG('t', "Entering SynchTest1\n");

    Thread *t1 = new Thread("producer 1");
    t1->Fork(Producer, 15);
    Thread *t2 = new Thread("producer 2");
    t2->Fork(Producer, 15);
    
    Thread *t3 = new Thread("consumer 1");
    t3->Fork(Consumer, 10);
    Thread *t4 = new Thread("consumer 2");
    t4->Fork(Consumer, 10);
    Thread *t5 = new Thread("consumer 3");
    t5->Fork(Consumer, 10);
    currentThread->Yield();


}
// philosopher
void
SynchTest2()
{
    PhiloInit();
    Thread *t1 = new Thread("philosopher 0");
    t1->Fork(Action, (int)philosophers[0]);
    Thread *t2 = new Thread("philosopher 1");
    t2->Fork(Action, (int)philosophers[1]);
    Thread *t3 = new Thread("philosopher 2");
    t3->Fork(Action, (int)philosophers[2]);
    Thread *t4 = new Thread("philosopher 3");
    t4->Fork(Action, (int)philosophers[3]);
    Thread *t5 = new Thread("philosopher 4");
    t5->Fork(Action, (int)philosophers[4]);
}
void
SynchTest3()
{

    Thread * r1 = new Thread("r1");
    Thread * r2 = new Thread("r2");
    Thread * r3 = new Thread("r3");
    Thread * w1 = new Thread("w1");
    Thread * w2 = new Thread("w2");

    r1->Fork(Reading,4);
    w1->Fork(Writing,6);
    r2->Fork(Reading,4);
    w2->Fork(Writing,8);
    r3->Fork(Reading,4);

}

void
SynchTest4()
{
    IntStatus old = interrupt->SetLevel(IntOff);
    char* name[barrier_count];
    for(int i=0;i<barrier_count;i++){
        name[i] = new char[8];
        for(int j =0;j<8;j++){
            name[i][j]="adder ?"[j];
        }
        name[i][6] = i+'0';
    } 
    Thread* t[barrier_count];
    for(int i=0;i<barrier_count;i++){
        //name[6]=i+'0';
        t[i] = new Thread((char*)name[i]);
        printf("%s\n",(char*)name[i]);
        t[i]->Fork(Add,10);
    }
    interrupt->SetLevel(old);
}

void
SynchTest()
{
    printf("Entering SynchTest.\n");
    switch (testnum) {
    case 1:
	SynchTest1();
	break;
    case 2:
    SynchTest2();
    break;
    case 3:
    SynchTest3();
    break;
    case 4:
    SynchTest4();
    break;
    default:
	printf("No test specified.\n");
	break;
    }
}
