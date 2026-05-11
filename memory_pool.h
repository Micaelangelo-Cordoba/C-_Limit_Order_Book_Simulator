#pragma once
#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H
#include <vector>
#include "Types.h"


class OrderPool { //this will create a preallocated memory pool, preventing 
    //us from constantly using the 'new' keyword to ask the OS to borrow memory from the heap

    private:
        std::vector<Order> arena; //Store millions of orders in a massive block of memory
        Order* free_head {nullptr}; //always points to the next available unused order

    public:
        OrderPool(size_t capacity);
        Order* allocate(); // No system calls o(1) allocation
        void free(Order* node); //push back to stack o(1) deletion.


};

inline OrderPool::OrderPool(size_t capacity) {
    arena.resize(capacity); //forces the OS to allocate capacity amount of memory for orders
    for (size_t i {}; i < capacity -1; ++i) {
        arena[i].next = &arena[i+1]; //Orders are linked together
    }
    free_head = &arena[0];
}

inline Order* OrderPool::allocate() { 
    if (!free_head) return nullptr;
    Order* node = free_head;
    free_head = free_head->next; //pop the front of the allocated block 
    //o(1) stack pop the node from the top of the chain, make it point to the next
    //to represent the new head.

    node->next = nullptr; //make sure it no longer points to this block of memory
    node->prev = nullptr;
    return node; //hand it to the matching engine!
}

inline void OrderPool::free(Order* node) { //in case of cancelled order or matched orders
    node->next = free_head; //o(1) stack push, push it back to our allocated
    //memory block, the head become our retired node. 
    free_head = node;
}


#endif