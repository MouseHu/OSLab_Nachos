#include "copyright.h"
#include "system.h"
#include "synch.h"
#ifndef BARRIER_H
#define BARRIER_H
#define barrier_count 6
class Barrier{
    private:
        int count;
        Lock* count_lock;
        Condition* cond;
    public:
        Barrier(int c);
        void Synch();
};

extern Barrier* barrier;
extern int count;

void Add(int n);
#endif