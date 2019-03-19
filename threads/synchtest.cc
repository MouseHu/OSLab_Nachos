//add by huhao used to test synchronize threads
#include "copyright.h"
#include "system.h"
#include "synchlist.h"
#include <string>
#include<sstream>
// prolist using lock 
SynchList* proList = new SynchList;
extern int testnum;
int list_count=0;
const int max_product = 8;


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
int Product::product_count=0;

void Producer(int n){
    //printf("producer here.\n");
    for(int i = 0;i<n;i++){
        
        IntStatus old = interrupt->SetLevel(IntOff);
        Product* product = new Product;
        while(list_count>=max_product){
            currentThread->Sleep();
        }
        proList->Append((void *)(product));
        list_count++;
        //product->setID(product_count++);
        printf("thread: %s, producing product: %d, product queue len:%d \n",currentThread->getName(), product->getID(),list_count);
        interrupt->SetLevel(old);
    }
}
void Consumer(int n){
    //printf("consumer here.\n");
    for(int i = 0;i<n;i++){
        IntStatus old = interrupt->SetLevel(IntOff);
        Product* product = (Product*)proList->Remove();
        list_count--;
        printf("thread: %s, consuming product: %d,product queue len:%d\n",currentThread->getName(), product->getID(),list_count);
        interrupt->SetLevel(old);
    }
    
}

// Producer Consumer test for semaphore
void
SynchTest1()
{
    DEBUG('t', "Entering SynchTest1\n");

    Thread *t1 = new Thread("producer 1");
    t1->Fork(Producer, 15);
    Thread *t2 = new Thread("producer 2");
    t2->Fork(Producer, 15);
    
    Thread *t3 = new Thread("consumer 1");
    t3->Fork(Consumer, 10);
    Thread *t4 = new Thread("consumer 2");
    t4->Fork(Consumer, 10);
    Thread *t5 = new Thread("consumer 3");
    t5->Fork(Consumer, 10);
    currentThread->Yield();


}

void
SynchTest2()
{
}
void
SynchTest3()
{
}
void
SynchTest4()
{
}

void
SynchTest()
{
    printf("Entering SynchTest.\n");
    switch (testnum) {
    case 1:
	SynchTest1();
	break;
    case 2:
    SynchTest2();
    break;
    case 3:
    SynchTest3();
    break;
    default:
	printf("No test specified.\n");
	break;
    }
}