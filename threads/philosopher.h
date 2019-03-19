#include "copyright.h"
#include "system.h"
#include "synch.h"
#ifndef PHILOSOPHER_H
#define PHILOSOPHER_H
#define num_philosopher 5
class Philosopher{
    private:
        bool eating;

        int id;

    public:
        bool left_chop;
        bool right_chop;
        Philosopher* left_neighbor;
        Philosopher* right_neighbor;
        Condition* cond;
        int chopnum;
        Philosopher(int id);
        ~Philosopher();
        void Eat();
        void Think();
        void PickChop();
        void SetNeighbor(Philosopher* l,Philosopher* r);
};
static void Eat(int p){
    ((Philosopher*)p)->Eat();
}
static void Think(int p){
    ((Philosopher*)p)->Think();
}
static void PickChop(int p){
    ((Philosopher*)p)->PickChop();
}
void Action(int p);
void PhiloInit();
extern Philosopher* philosophers[num_philosopher];
extern Lock* table;
extern bool chops[num_philosopher];
#endif