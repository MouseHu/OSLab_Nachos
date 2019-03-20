#include "copyright.h"
#include "system.h"
#include "synch.h"
#ifndef WRLOCK_H
#define WRLOCK_H
class WRlock{
    private:
        Semaphore* wrlock;
        Lock* rclock;
        int reader_count;
    public:
        WRlock();
        ~WRlock();
        void ReadAcquire();
        void ReadRelease();
        void WriteAcquire();
        void WriteRelease();
        int ReaderCount(){return reader_count;};
};

void Reading(int p);
void Writing(int p);

extern WRlock* wrlock;
extern int file;
#endif