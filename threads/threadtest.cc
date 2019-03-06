// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"

// testnum is set in main.cc
int testnum = 1;

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread: %s userID:%d threadID:%d, looped %d times\n", currentThread->getName(), currentThread->getUserID(), currentThread->getThreadID(), num);
        currentThread->Yield();
    }
}

//add by huhao
void
PrintAllThreads(int which)
{
    printf("TS Command, Listing All Threads:\n");
    IntStatus oldlevel = interrupt->SetLevel(IntOff);
    currentThread->PrintStatus();
    List * ready_threads = scheduler->GetReadyList();
    ListElement *threads_iter = ready_threads->GetFirst();
    while(threads_iter!= NULL){
        Thread *t = (Thread *)threads_iter->item;
        t->PrintStatus();
        threads_iter= threads_iter->next;
    }
    printf("\n");
    currentThread->Yield();
    interrupt->SetLevel(oldlevel);
}

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");
    t->Fork(SimpleThread, 0);


    SimpleThread(0);
}

//add by huhao
//----------------------------------------------------------------------
// ThreadTest2
// 	Test threadIDs and userIDs.
//	
//----------------------------------------------------------------------
void
ThreadTest2()
{
    DEBUG('t', "Entering ThreadTest2");

    Thread *t1 = new Thread("forked thread 1");
    Thread *t2 = new Thread("forked thread 2");
    t1->Fork(SimpleThread, 1);
    t2->Fork(SimpleThread,1);

    SimpleThread(0);
}
//----------------------------------------------------------------------
// ThreadTest3
// 	Test max thread numbers.
//	
//----------------------------------------------------------------------

void
ThreadTest3()
{
    DEBUG('t', "Entering ThreadTest3");
    for(int i =0;i<MAX_THREAD+1;i++){
        Thread *t = new Thread("forked thread");
        printf("*** thread: %s userID:%d threadID:%d\n", t->getName(), t->getUserID(), t->getThreadID());

    }
}

//----------------------------------------------------------------------
// ThreadTest3  
// 	Test TS command
//	
//----------------------------------------------------------------------

void
ThreadTest4()
{
    DEBUG('t', "Entering ThreadTest4");
    Thread *t1 = new Thread("forked thread 1");
    Thread *t2 = new Thread("forked thread 2");
    Thread *t3 = new Thread("forked thread 2");
    t1->Fork(PrintAllThreads, 1);
    t2->Fork(PrintAllThreads,1);
    t3->Fork(PrintAllThreads,1);
}
//add by huhao
//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    switch (testnum) {
    case 1:
	ThreadTest1();
	break;
    case 2:
    ThreadTest2();
    break;
    case 3:
    ThreadTest3();
    case 4:
    ThreadTest4();
    default:
	printf("No test specified.\n");
	break;
    }
}

