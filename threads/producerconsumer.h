#include "copyright.h"
#include "system.h"
#include "synchlist.h"
#ifndef PRODUCER_CONSUMER_H
#define PRODUCER_CONSUMER_H
extern int list_count;
extern const int max_product;
extern SynchList* proList;

class Product{
    private:
        int product_id;
        static int product_count;
        
        
    public:
        Product(){
            product_id = product_count++;
        }
        int getID(){
           return product_id; 
        }
        
};

void Consumer(int n);
void Producer(int n);

#endif