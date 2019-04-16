// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
#include "../machine/machine.h"
#include "copyright.h"
#include "system.h"
#include "syscall.h"
int TLBAlgoType =0;

//add by huhao 
int invalidTLB(){
    int insert_index = -1;
    for(int i =0;i<TLBSize;i++){
        if(machine->tlb[i].valid ==FALSE){
            insert_index = i;
            break;
        }
    }
    return insert_index;
}

int FIFO(){
    int insert_index = invalidTLB();
    if(insert_index!=-1)
        return insert_index;
    for(int i =TLBSize-1;i>0;i--){
        machine->tlb[i]=machine->tlb[i-1];
    }
    return 0;
}

int LRU(){
    int insert_index = invalidTLB();
    if(insert_index!=-1)
        return insert_index;
    int max_time = machine->tlb[0].timestamp;
    insert_index = 0;
    for(int i = 1;i<TLBSize;i++){
        if(machine->tlb[i].timestamp>max_time){
            insert_index=i;
            max_time = machine->tlb[i].timestamp;
        }
    }
    return insert_index;
}
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
    
    int type = machine->ReadRegister(2);
    //rintf("dealing with exception,%d %d",which,type);
    if ((which == SyscallException) && (type == SC_Halt)) {
	DEBUG('a', "Shutdown, initiated by user program.\n");
    
   	interrupt->Halt();
    } 
    else if ((which == SyscallException) && (type == SC_Exit)){
        printf("program exit.\n");
        machine->deleteMem();
        interrupt->Halt();
    }
    else if(which==int(PageFaultException)){
        if(machine->tlb!=NULL){
            // TLB fault
            int badAddr = machine->registers[BadVAddrReg];
            unsigned int vpn = badAddr/PageSize;
            int insert_index = FIFO();
            switch(TLBAlgoType){
                case 0:
                    insert_index = MAX(0,invalidTLB());
                    break;
                case 1:
                    insert_index = FIFO();
                    break;
                case 2:
                    insert_index = LRU();
                    break;
                default:
                    insert_index = MAX(0,invalidTLB());
            } 
            machine->tlb[insert_index].valid = TRUE;
            machine->tlb[insert_index].virtualPage = vpn;
            machine->tlb[insert_index].physicalPage = machine->pageTable[vpn].physicalPage; 
            machine->tlb[insert_index].use = FALSE;
            machine->tlb[insert_index].dirty = FALSE;
            machine->tlb[insert_index].readOnly = FALSE;
            machine->tlb[insert_index].timestamp = 0;
        }
        else{
            //Page Fault
        }
        // machine->registers[PrevPCReg] = machine->registers[PCReg];	// for debugging, in case we
		// 				// are jumping into lala-land
        // machine->registers[PCReg] = machine->registers[NextPCReg];
        // machine->registers[NextPCReg] = machine->registers[NextPCReg]+4;
    }
    else{
        printf("Unexpected user mode exception %d %d\n", which, type);
        ASSERT(FALSE);
    }
}


