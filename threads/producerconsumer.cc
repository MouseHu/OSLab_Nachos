#include "producerconsumer.h"
int list_count=0;
const int max_product = 8;
SynchList* proList = new SynchList;
int Product::product_count=0;

void Producer(int n){

    for(int i = 0;i<n;i++){
        IntStatus old = interrupt->SetLevel(IntOff);
        Product* product = new Product;
        while(list_count>=max_product){
            currentThread->Sleep();
        }
        proList->Append((void *)(product));
        list_count++;
        printf("thread: %s, producing product: %d, product queue len:%d \n",currentThread->getName(), product->getID(),list_count);
        interrupt->SetLevel(old);
    }
}

void Consumer(int n){

    for(int i = 0;i<n;i++){
        IntStatus old = interrupt->SetLevel(IntOff);
        Product* product = (Product*)proList->Remove();
        list_count--;
        printf("thread: %s, consuming product: %d,product queue len:%d\n",currentThread->getName(), product->getID(),list_count);
        interrupt->SetLevel(old);
    }
    
}