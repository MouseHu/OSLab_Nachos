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
int PageTableAlgoType =0;

//add by huhao 
int invalid(TranslationEntry* entry,int size){
    int insert_index = -1;
    for(int i =0;i<size;i++){
        if(entry[i].valid ==FALSE){
            insert_index = i;
            break;
        }
    }
    return insert_index;
}

int FIFO(TranslationEntry* entry,int size){
    int insert_index = invalid(entry,size);
    if(insert_index!=-1)
        return insert_index;
    for(int i =size-1;i>0;i--){
        entry[i]=entry[i-1];
    }
    return 0;
}

int LRU(TranslationEntry* entry,int size){
    int insert_index = invalid(entry,size);
    if(insert_index!=-1)
        return insert_index;
    int max_time = entry[0].timestamp;
    insert_index = 0;
    for(int i = 1;i<size;i++){
        if(entry[i].timestamp>max_time){
            insert_index=i;
            max_time = entry[i].timestamp;
        }
    }
    return insert_index;
}


int DealPageFault(TranslationEntry* entry){

    int badAddr = machine->registers[BadVAddrReg];
    unsigned int vpn = badAddr/PageSize;
    int exchange_index = -1;
    if( machine->pageMap->NumClear() > 0 ){

        entry->valid = true;
        entry->physicalPage = machine->pageMap->Find();
        //PmEntry[ userEntry->physicalPage ] = userEntry;
        exchange_index = entry->physicalPage;
        //printf("here.%d\n",exchange_index);
        memcpy(&(machine->mainMemory[entry->physicalPage * PageSize ] ),
        &(machine->virtualMemory[entry->virtualPage * PageSize]),PageSize );
    }
    else{

        switch(PageTableAlgoType){
            case 0:
                exchange_index = 0;//MAX(0,invalid(machine->pageTable,machine->pageTableSize));
                break;
            case 1:
                exchange_index = FIFO(machine->pageTable,machine->pageTableSize);
                break;
            case 2:
                exchange_index = LRU(machine->pageTable,machine->pageTableSize);
                break;
            default:
                exchange_index = MAX(0,invalid(machine->pageTable,machine->pageTableSize));
        }
        for(int i=0;i<machine->pageTableSize;i++){
            if(machine->pageTable[i].physicalPage == exchange_index){
                machine->pageTable[i].valid=FALSE;
                if(machine->pageTable[i].dirty){
                    memcpy(&(machine->virtualMemory[machine->pageTable[i].virtualPage * PageSize]),
                    &(machine->mainMemory[exchange_index * PageSize ] ),PageSize );
                }
            }
        }
        memcpy(&(machine->mainMemory[exchange_index * PageSize ] ),
        &(machine->virtualMemory[entry->virtualPage * PageSize]),PageSize );
        machine->pageTable[vpn].valid = TRUE;
        machine->pageTable[vpn].virtualPage = vpn;
        machine->pageTable[vpn].physicalPage = exchange_index; 
        machine->pageTable[vpn].use = FALSE;
        machine->pageTable[vpn].dirty = FALSE;
        machine->pageTable[vpn].readOnly = FALSE;
        machine->pageTable[vpn].timestamp = 0;
    }
    printf("Page Fault.exchange: %d\n",exchange_index);
    return exchange_index;
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
        IntStatus old = interrupt->SetLevel(IntOff);
        currentThread->Sleep();//never awake
        interrupt->SetLevel(old);
    }
    else if ((which == SyscallException) && (type == SC_Yield)){
        printf("program yield.\n");
        // int nextPC = machine->ReadRegister(NextPCReg);
        // machine->WriteRegister(PCReg,nextPC);
        currentThread->Yield();

    }
    else if(which==int(PageFaultException)){
        int badAddr = machine->registers[BadVAddrReg];
        unsigned int vpn = badAddr/PageSize;
        if(machine->tlb!=NULL){
            // TLB MISS
            
            int insert_index = -1;
            switch(TLBAlgoType){
                case 0:
                    insert_index = MAX(0,invalid(machine->tlb,TLBSize));
                    break;
                case 1:
                    insert_index = FIFO(machine->tlb,TLBSize);
                    break;
                case 2:
                    insert_index = LRU(machine->tlb,TLBSize);
                    break;
                default:
                    insert_index = MAX(0,invalid(machine->tlb,TLBSize));
            }
            //printf("test:%d,%d",vpn,machine->pageTable[vpn].valid);
            if(machine->pageTable[vpn].valid==FALSE ){
                //Page Fault
                int exchangePage = DealPageFault(&(machine->pageTable[vpn]));
                if(exchangePage!=-1){
                    // change TLB table
                    for(int i=0;i<TLBSize;i++){
                        if(machine->tlb[i].physicalPage==exchangePage)
                            machine->tlb[i].valid = FALSE;
                    }
                }
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
            DealPageFault(&(machine->pageTable[vpn]));
        }
    }
    else{
        printf("Unexpected user mode exception %d %d\n", which, type);
        ASSERT(FALSE);
    }
}


