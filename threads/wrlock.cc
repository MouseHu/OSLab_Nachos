#include "wrlock.h"

WRlock::WRlock(){
    reader_count = 0;
    wrlock = new Semaphore("wrlock",1);
    rclock = new Lock("rclock");
}

WRlock::~WRlock(){
    delete wrlock;
    delete rclock;
}

void WRlock::ReadAcquire(){
    IntStatus old =interrupt->SetLevel(IntOff);
    rclock->Acquire();
    reader_count++;
    DEBUG('t',"***********%d***********\n",reader_count);
    if(reader_count==1){
        rclock->Release();
        wrlock->P();
    }
    else{
        rclock->Release();
    }
    interrupt->SetLevel(old);
}
void WRlock::ReadRelease(){
    IntStatus old =interrupt->SetLevel(IntOff);
    rclock->Acquire();
    reader_count--;
    DEBUG('t',"***********%d***********\n",reader_count);
    if(reader_count==0){
        wrlock->V();
        rclock->Release();
    }
    else{
        rclock->Release();
    }
    interrupt->SetLevel(old);
    
}

void WRlock::WriteAcquire(){
    IntStatus old =interrupt->SetLevel(IntOff);
    wrlock->P();

    interrupt->SetLevel(old);
        
}
void WRlock::WriteRelease(){
    IntStatus old =interrupt->SetLevel(IntOff);
    wrlock->V();
    interrupt->SetLevel(old);
}

WRlock* wrlock = new WRlock();
int file =1;

void Reading(int p){
    
    for(int i=0;i<p;i++){
        wrlock->ReadAcquire();
        printf("thread: %s reading,\treader count:%d\tfile:%d\n",currentThread->getName(),wrlock->ReaderCount(),file);
        for(int j =0;j<3;j++){
            IntStatus old = interrupt->SetLevel(IntOff);
            interrupt->SetLevel(old);
        }
        //
        wrlock->ReadRelease();
        printf("thread: %s idling,\treader count:%d\n",currentThread->getName(),wrlock->ReaderCount());
        for(int j =0;j<10;j++){
            IntStatus old = interrupt->SetLevel(IntOff);
            interrupt->SetLevel(old);
        }
    }
    
    

}
void Writing(int p){
    
    for(int i=0;i<p;i++){
        wrlock->WriteAcquire();
        file = i;
        printf("thread: %s writng:%d,reader count:%d\n",currentThread->getName(),file,wrlock->ReaderCount());
        for(int j =0;j<3;j++){
            IntStatus old = interrupt->SetLevel(IntOff);
            interrupt->SetLevel(old);
        }
        wrlock->WriteRelease();
        printf("thread: %s idling\n",currentThread->getName());
        for(int j =0;j<10;j++){
            IntStatus old = interrupt->SetLevel(IntOff);
            interrupt->SetLevel(old);
        }
    }
    
    
}

