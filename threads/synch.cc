// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
//modified by huhao
Lock::Lock(char* debugName) {
    name = debugName;
    locked = false;
    occupingThread = NULL;
    queue = new List;
}
Lock::~Lock() {
    delete queue;
}
void Lock::Acquire() {
    IntStatus old =interrupt->SetLevel(IntOff);
    if(isHeldByCurrentThread()){
        DEBUG('t',"lock %s already acquired\n",name);
        return;
    }
    while(locked){
        queue->Append((void*)currentThread);
        currentThread->Sleep();
    }
    DEBUG('t',"lock %s acquired\n",name);
    locked = TRUE;
    occupingThread = currentThread;
    interrupt->SetLevel(old);
}
void Lock::Release() {
    IntStatus old = interrupt->SetLevel(IntOff);
    if(!isHeldByCurrentThread()){
        return;
    }else if(locked==false){
        DEBUG('t',"lock %s already released\n",name);
        return;
    }
    locked = FALSE;
    occupingThread = NULL;
    DEBUG('t',"lock %s released\n",name);
    if(!queue->IsEmpty()){
        Thread* awakeThread = (Thread*)queue->Remove();
        DEBUG('t',"lock awaking thread %s",awakeThread->getName());
        scheduler->ReadyToRun(awakeThread);
    }
    interrupt->SetLevel(old);
}
//bool Lock::isHeldBy
bool Lock::isHeldByCurrentThread(){
    if(occupingThread==NULL)
        return FALSE;
    DEBUG('t',"current thread: %s occpuingThread:%s\n",currentThread->getName(),((Thread*)occupingThread)->getName());
    return ((void*)occupingThread==(void *)currentThread);
};

Condition::Condition(char* debugName) { 
    name = debugName;
    queue = new List;
}
Condition::~Condition() {
    delete queue;
 }
void Condition::Wait(Lock* conditionLock) {
    IntStatus old = interrupt->SetLevel(IntOff);
    conditionLock->Release();
    queue->Append((void*)currentThread);
    currentThread->Sleep();
    //printf("thread:%s what?",currentThread->getName());
    conditionLock->Acquire();
    interrupt->SetLevel(old);
}
void Condition::Signal(Lock* conditionLock) {
    IntStatus old = interrupt->SetLevel(IntOff);
    //conditionLock->Acquire();
    ASSERT(conditionLock->isHeldByCurrentThread())
    if(!queue->IsEmpty()){
        Thread* awakenThread = (Thread*)queue->Remove();
        printf("awaking %s\n",awakenThread->getName());
        scheduler->ReadyToRun(awakenThread);
    }
    conditionLock->Release();
    interrupt->SetLevel(old);
 }
void Condition::Broadcast(Lock* conditionLock) {
    IntStatus old = interrupt->SetLevel(IntOff);
    ASSERT(conditionLock->isHeldByCurrentThread())
    while(!queue->IsEmpty()){
        Thread* awakenThread = (Thread*)queue->Remove();
        printf("awaking %s\n",awakenThread->getName());
        scheduler->ReadyToRun(awakenThread);
    }
    conditionLock->Release();
    interrupt->SetLevel(old);
 }
