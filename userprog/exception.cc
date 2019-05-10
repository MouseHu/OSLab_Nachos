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


int TLBAlgoType =3 ;
int PageTableAlgoType =2;

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

int DealReversePageFault(TranslationEntry* reversedPageTable){

    DEBUG('a',"entering deal reversed page fault.\n");
    int badAddr = machine->registers[BadVAddrReg];
    unsigned int vpn = badAddr/PageSize;
    int exchange_index = -1;
    if( machine->pageMap->NumClear() > 0 ){
        int physicalPage = machine->pageMap->Find();
        //printf("here.%d\n",exchange_index);
        TranslationEntry* entry = &(reversedPageTable[physicalPage]);
        //printf("here.%d\n",reversedPageTable);
        entry->valid = true;
        entry->physicalPage = physicalPage;
        //printf("find: %d %d\n",entry->physicalPage,entry->virtualPage);
        //PmEntry[ userEntry->physicalPage ] = userEntry;
        exchange_index = physicalPage;
        entry->virtualPage = vpn;
        //printf("here.%d\n",(int*)machine->virtualMemory);
        memcpy(&(machine->mainMemory[entry->physicalPage * PageSize ] ),
        &(machine->virtualMemory[entry->virtualPage * PageSize]),PageSize );
        entry->use = FALSE;
        entry->dirty = FALSE;
        entry->readOnly = FALSE;
        entry->timestamp=0;
    }
    else{

        switch(PageTableAlgoType){
            case 0:
                exchange_index = 0;//MAX(0,invalid(machine->pageTable,machine->pageTableSize));
                break;
            case 1:
                exchange_index = FIFO(machine->reversedPageTable,NumPhysPages);
                break;
            case 2:
                exchange_index = LRU(machine->reversedPageTable,NumPhysPages);
                break;
            default:
                exchange_index = MAX(0,invalid(machine->reversedPageTable,NumPhysPages));
        }
        printf("%d\n",exchange_index);
        if((machine->reversedPageTable[exchange_index].valid==TRUE)){
            if(machine->reversedPageTable[exchange_index].dirty){
                    memcpy(&(machine->virtualMemory[machine->reversedPageTable[exchange_index].virtualPage * PageSize]),
                    &(machine->mainMemory[exchange_index * PageSize ] ),PageSize );
                }
        }

        memcpy(&(machine->mainMemory[exchange_index * PageSize ] ),
        &(machine->virtualMemory[vpn * PageSize]),PageSize );
        machine->reversedPageTable[exchange_index].valid = TRUE;
        machine->reversedPageTable[exchange_index].virtualPage = vpn;
        machine->reversedPageTable[exchange_index].physicalPage = exchange_index; 
        machine->reversedPageTable[exchange_index].use = FALSE;
        machine->reversedPageTable[exchange_index].dirty = FALSE;
        machine->reversedPageTable[exchange_index].readOnly = FALSE;
        machine->reversedPageTable[exchange_index].timestamp = 0;
    }
    printf("(ReversePageTable) Page Fault.exchanging physical page: %d\n",exchange_index);
    return exchange_index;
}

int DealPageFault(TranslationEntry* entry){
    DEBUG('a',"entering deal page fault.\n");
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
        entry->use = FALSE;
        entry->dirty = FALSE;
        entry->readOnly = FALSE;
        entry->timestamp=0;
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
        printf("%d\n",exchange_index);
        for(int i=0;i<machine->pageTableSize;i++){
            if((machine->pageTable[i].physicalPage == exchange_index)&&(machine->pageTable[i].valid==TRUE)){
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
    printf("Page Fault. exchanging physical page: %d\n",exchange_index);
    return exchange_index;
}
char* ReadFileName(){
    char* name = new char[20];
    //char name[20];
    int name_addr = machine->ReadRegister(4);
    int data=100;
    int i =0;
    while(char(data)!='\0'){
        while(!machine->ReadMem(name_addr+i,1,&data)); 
        *(name+i)=(char)data;
        i++;    
    }
    printf("%s\n",name);
    return name;
}
void CreateHandler(){
    printf("SC Create\n");
    char* name =ReadFileName();
    printf("%s\n",name);
    bool success = fileSystem->Create(name,128);
    //machine->WriteRegister(2,int(success));
    machine->PCAdvance();
}

void OpenHandler(){
    printf("SC Open\n");
    char* name = ReadFileName();
    int file = (int)fileSystem->Open(name);
    machine->WriteRegister(2,file);
    printf("open file %s,%d\n",name,file);
    machine->PCAdvance();
}

void CloseHandler(){
    printf("SC Close\n");
    int file = (int)(machine->ReadRegister(4));
    OpenFile *openfile = (OpenFile*)file;
    //printf("close file %d\n",openfile->Length());
    //delete openfile;
    printf("close file %d\n",(int)openfile);
    machine->PCAdvance();
}

void ReadHandler(){
    printf("SC Read\n");
    int buffer = (machine->ReadRegister(4));
    int size = (int)(machine->ReadRegister(5));
    OpenFile* file = (OpenFile*)(machine->ReadRegister(6));
    char* content = new char[size];
    int success = file->Read(content,size);
    int data =0;
    for(int i =0;i<size;i++){
        while(!machine->WriteMem(buffer+i,1,int(content[i])));
    }
    machine->WriteRegister(2,int(success));
    machine->PCAdvance();
}

void WriteHandler(){
    printf("SC Write\n");
    int buffer = machine->ReadRegister(4);
    int size = (int)(machine->ReadRegister(5));
    OpenFile* file = (OpenFile*)(machine->ReadRegister(6));
    char* content = new char[size];
    int data;
    for(int i =0;i<size;i++){
        while(!machine->ReadMem(buffer+i,1,&data));
        content[i]=char(data);
    }
    int success = file->Write(content,size);
    //machine->WriteRegister(2,success);
    machine->PCAdvance();
}

void YieldHandler(){
    printf("SC Yield\n");
    machine->PCAdvance();
    currentThread->Yield();
}

void JoinHandler(){
    printf("SC Join\n");
    int threadID = machine->ReadRegister(4);
    while(threads_occupied[threadID]){
        //printf("%d\n",currentThread->getThreadID());
        currentThread->Yield();
    }
    machine->PCAdvance();
}

void SuspendHandler(){
    printf("program suspended (yield).\n");
    machine->suspendCurrentThread();
    currentThread->Suspend();
    machine->PCAdvance();
}
void ExitHandler(){
    printf("SC Exit\n");
    int status = machine->ReadRegister(4);
    printf("program exit with status: %d\n",status);
    machine->PCAdvance();
    currentThread->Finish();
}
void ExecRun(int executable){
    printf("Running user process:%s\n",currentThread->getName());
    AddrSpace *space; 
    space = new AddrSpace((OpenFile*)executable);     
    currentThread->space = space;
    delete (OpenFile*)executable;			// close file
    space->InitRegisters();		// set the initial register values
    space->RestoreState();		// load page table register
    printf("in exec run\n");
    machine->Run();
    currentThread->Finish();
}

void ForkRun(int func_pc){
    currentThread->space->InitRegisters();
    currentThread->space->RestoreState();
    machine->WriteRegister(PCReg,func_pc);
    machine->WriteRegister(NextPCReg,func_pc+4);

    machine->Run();
    currentThread->Finish();
}
void ExecHandler(){
    printf("SC Exec\n");
    char* name = ReadFileName();
    Thread* thread = new Thread("Exec");
    OpenFile *executable = fileSystem->Open(name);
    thread->Fork(ExecRun,(int)executable);
    machine->WriteRegister(2,thread->getThreadID());
    machine->PCAdvance();
}

void ForkHandler(){
    printf("SC Fork\n");
    int func_pc = machine->ReadRegister(4);
    Thread* thread = new Thread("Fork");
    thread->space=currentThread->space;
    thread->Fork(ForkRun,func_pc);
    machine->PCAdvance();
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
    DEBUG('a',"entering exception handler.\n");
    int type = machine->ReadRegister(2);
    //rintf("dealing with exception,%d %d",which,type);
    if(which==SyscallException){
        switch (type)
        {
            case SC_Halt:
                printf("SC Halt\n");
                DEBUG('a', "Shutdown, initiated by user program.\n");
                interrupt->Halt();
                break;
            case SC_Exit:
                ExitHandler();
                break;
            case SC_Yield:
                YieldHandler();
                break;
            case SC_Fork:
                ForkHandler();
                break;
            case SC_Join:
                JoinHandler();
                break;
            case SC_Create:
                CreateHandler();
                break;
            case SC_Open:
                OpenHandler();
                break;
            case SC_Close:
                CloseHandler();
                break;
            case SC_Read:
                ReadHandler();
                break;
            case SC_Write:
                WriteHandler();
                break;
            case SC_Exec:
                ExecHandler();
                break;
            default:
                break;
        }
    }
    else if(which==int(PageFaultException)){
        int badAddr = machine->registers[BadVAddrReg];
        unsigned int vpn = (unsigned) badAddr/PageSize;
        int exchangePage=-1;
        if(machine->tlb!=NULL){
            // TLB MISS
            //printf("tlb miss\n");
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
                case 3:
                    insert_index = 0;
                    break;
                default:
                    insert_index = MAX(0,invalid(machine->tlb,TLBSize));
            }
            //printf("test:%d,%d\n",vpn,badAddr);
            #ifdef REVERSE_PAGETABLE
            exchangePage == -1;
            for(int i=0;i<NumPhysPages;i++){
                if((machine->reversedPageTable[i].valid==TRUE)&&(machine->reversedPageTable[i].virtualPage == vpn)){
                    exchangePage = i;
                    break;
                }
            }
            //printf("test2:%d\n",exchangePage);
            if(exchangePage==-1){
                //Page Fault
                exchangePage = DealReversePageFault(machine->reversedPageTable);
                if(exchangePage!=-1){
                    // change TLB table
                    for(int i=0;i<TLBSize;i++){
                        if(machine->tlb[i].physicalPage==exchangePage)
                            machine->tlb[i].valid = FALSE;
                    }
                }
            }
            #else

            if(machine->pageTable[vpn].valid==FALSE ){
                //Page Fault
                exchangePage = DealPageFault(&(machine->pageTable[vpn]));
                //printf("whta?%d\n",(machine->pageTable[vpn]).physicalPage);
                if(exchangePage!=-1){
                    // change TLB table
                    for(int i=0;i<TLBSize;i++){
                        if(machine->tlb[i].physicalPage==exchangePage)
                            machine->tlb[i].valid = FALSE;
                    }
                }

            }
            else{
                exchangePage=machine->pageTable[vpn].physicalPage;
            }
            #endif
            //printf("insert index %d, phy page %d",insert_index,exchangePage);
            machine->tlb[insert_index].valid = TRUE;
            machine->tlb[insert_index].virtualPage = vpn;
            machine->tlb[insert_index].physicalPage = exchangePage; 
            machine->tlb[insert_index].use = FALSE;
            machine->tlb[insert_index].dirty = FALSE;
            machine->tlb[insert_index].readOnly = FALSE;
            machine->tlb[insert_index].timestamp = 0;
            //printf("test:%d,%d %d %d\n",insert_index,vpn,machine->pageTable[vpn].physicalPage,machine->pageTable[vpn].valid);
            //int nextPC = machine->ReadRegister(NextPCReg);
            //machine->WriteRegister(NextPCReg,machine->registers[PCReg]);
            //machine->WriteRegister(PCReg,machine->registers[PrevPCReg]);
            
            
        }
        else{
            //Page Fault
            #ifdef REVERSE_PAGETABLE
            DealReversePageFault(machine->reversedPageTable);
            #else
            DealPageFault(&(machine->pageTable[vpn]));
            #endif
        }
    }
    else{
        printf("Unexpected user mode exception %d %d\n", which, type);
        ASSERT(FALSE);
    }
}


