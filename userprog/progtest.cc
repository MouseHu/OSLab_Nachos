// progtest.cc 
//	Test routines for demonstrating that Nachos can load
//	a user program and execute it.  
//
//	Also, routines for testing the Console hardware device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
// #include "console.h"
#include "synchconsole.h"
#include "addrspace.h"
#include "synch.h"

void RunUser(int dummy){
    printf("Running user process:%s\n",currentThread->getName());
    //machine->pageMap->Print();
    machine->Run();
    
}
//----------------------------------------------------------------------
// StartProcess
// 	Run a user program.  Open the executable, load it into
//	memory, and jump to it.
//----------------------------------------------------------------------

void
StartProcess(char *filename)
{
    //Thread* thread = new Thread("Copied thread");
    printf("start process.\n");
    OpenFile *executable = fileSystem->Open(filename);
    //OpenFile *executable2 = fileSystem->Open(filename);

    AddrSpace *space;
    //AddrSpace *space2;

    if (executable == NULL) {
	printf("Unable to open file %s\n", filename);
	return;
    }
    space = new AddrSpace(executable);  
    //space2 = new AddrSpace(executable2);    
    currentThread->space = space;
    //thread->space = space2;
    //thread->Fork(RunUser,1);

    delete executable;			// close file

    space->InitRegisters();		// set the initial register values
    space->RestoreState();		// load page table register

    
    printf("program: %s start\n",currentThread->getName());
    machine->Run();			// jump to the user progam
    ASSERT(FALSE);			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}

void
Start2Processes(char *filename)
{
    Thread* thread = new Thread("Copied thread");
    
    OpenFile *executable = fileSystem->Open(filename);
    OpenFile *executable2 = fileSystem->Open(filename);

    AddrSpace *space;
    AddrSpace *space2;

    if (executable == NULL) {
	printf("Unable to open file %s\n", filename);
	return;
    }
    space = new AddrSpace(executable);  
    space2 = new AddrSpace(executable2);    
    currentThread->space = space;
    thread->space = space2;
    thread->Fork(RunUser,1);

    delete executable;			// close file
    delete executable2;
    space2->InitRegisters();		// set the initial register values
    space2->RestoreState();		// load page table register
    currentThread->Yield();

    space->InitRegisters();		// set the initial register values
    space->RestoreState();		// load page table register

    
    printf("program: %s start\n",currentThread->getName());
    machine->Run();			// jump to the user progam
    ASSERT(FALSE);			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}
// Data structures needed for the console test.  Threads making
// I/O requests wait on a Semaphore to delay until the I/O completes.

// static Console *console;
static SynchConsole *console;
static Semaphore *readAvail;
static Semaphore *writeDone;

//----------------------------------------------------------------------
// ConsoleInterruptHandlers
// 	Wake up the thread that requested the I/O.
//----------------------------------------------------------------------

static void ReadAvail(int arg) { readAvail->V(); }
static void WriteDone(int arg) { writeDone->V(); }

//----------------------------------------------------------------------
// ConsoleTest
// 	Test the console by echoing characters typed at the input onto
//	the output.  Stop when the user types a 'q'.
//----------------------------------------------------------------------

void 
ConsoleTest (char *in, char *out)
{
    char ch;

    // console = new Console(in, out, ReadAvail, WriteDone, 0);
    console = new SynchConsole(in, out);
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);
    
    for (;;) {
    // console->ReadAvail();
	// readAvail->P();		// wait for character to arrive
	ch = console->GetChar();
	console->PutChar(ch);	// echo it!
    // console->WriteDone();
	// writeDone->P() ;        // wait for write to finish
	if (ch == 'q') return;  // if q, quit
    }
}
