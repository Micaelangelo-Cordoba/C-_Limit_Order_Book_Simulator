#pragma once
#include <cstdint>
#include <unordered_map>
#include <map>
#include <vector>
#include <iostream>

struct PriceLevel;

//Order Node enforces it to fit nicely with CPU cache lines which are ~64 bytes
//makes them fit contiguously in memory
struct alignas(32) Order {
uint64_t id; //identifier
uint64_t price; //int price
uint64_t timestamp; //nanoseconds since start enforces time-priority tie breakers
uint32_t size; //quantity of shares 

Order* next {nullptr}; //pointer to next Order in the time-queue
Order* prev {nullptr}; //pointer to previous order

PriceLevel* parent_level = nullptr;
};

struct PriceLevel {  //All orders are grouped by price .
    Order* head {nullptr};
    Order* tail {nullptr};
    //this manages the case where there are multiple order with the same price, 
    //but different timestamps!
    uint64_t price;
    uint32_t total_volume = 0;
  
    bool is_bid;
};

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



//the engine 
class LimitOrderBook {
private:
    OrderPool memory_pool;

    //Allows for O(1) access of Orders. There are many realistic cases where 
    //orders get deleted before they get matched, so this hash map allows us to instantly access
    //delete them without having to traverse through the linked lists of them!
    std::unordered_map<uint64_t, Order*> order_map;


    //Use std::map for price priority of bids and asks (Binary Tree)
    std::map<uint64_t, PriceLevel, std::greater<uint64_t>> bids;
    //std::greater implies highest-to-lowest bids (High bids take higher priority)
    //std::less implies lowest-to-highest asks (low asks take higher priority)
    std::map<uint64_t, PriceLevel, std::less<uint64_t>> asks;

    public:
//memory pool will be initialized with 1 million empty Order structs. We will use this pool
//over time instead of constantly using the 'new' keyword 
    LimitOrderBook() : memory_pool {5'000'000} {}

    void add_buy_limit_order(uint64_t id, uint64_t price, uint32_t size, uint64_t ts);


    void add_sell_limit_order(uint64_t id, uint64_t price, uint32_t size, uint64_t ts);

    void cancel_order(uint64_t id); //cancel orders in o(1) based on id.

    void execute_market_buy(uint32_t size);
    
    void execute_market_sell(uint32_t size);

};
    
inline void LimitOrderBook::add_buy_limit_order(uint64_t id, uint64_t price, uint32_t size, uint64_t ts) {
        Order* buy_order = memory_pool.allocate(); //grab from our pre allocated mem block

        buy_order->id = id;
        buy_order->price = price;
        buy_order->size = size;
        buy_order->timestamp = ts;

        //find the price level based on the price o(logn) ->std::map

        PriceLevel& plevel = bids[price];
        buy_order->parent_level = &plevel;
        plevel.total_volume += size;
        plevel.is_bid = true;

        if (!plevel.head) {
            plevel.head = buy_order;
            plevel.tail = buy_order; //having a head and tail reduces traversal incase of insertion
        } else {
            plevel.tail->next = buy_order;  //if new orders have the same price, 
            //the never orders will be in the tail of the linked list that represents
            //a FIFO queue.
            buy_order->prev = plevel.tail;
            plevel.tail = buy_order;
        }

        order_map[id] = buy_order; //store the order in the hash map so when we need to access
        //it regardless of priority of queue we can access it in constant o(1) lookup rather
        //than traversing through the containers.
    }

inline void LimitOrderBook::add_sell_limit_order(uint64_t id, uint64_t price, uint32_t size, uint64_t ts) {
        Order* ask_order = memory_pool.allocate(); //instead of using 'new'
        ask_order->id = id; 
        ask_order->price = price;
        ask_order->size = size;
        ask_order->timestamp = ts;

        PriceLevel& plevel = asks[price]; //each price has its respective priceLevel struct
        //that ensures time priority of similar orders are enforced. 

        ask_order->parent_level = &plevel;
        plevel.total_volume += size; // how many asks with that level
        plevel.is_bid = false;


        if (!plevel.head) { //will happen when just created
            plevel.head = ask_order;
            plevel.tail = ask_order;

        } else { //add to the back
            plevel.tail->next = ask_order;
            ask_order->prev = plevel.tail;
            plevel.tail = ask_order;
        }
        order_map[id] = ask_order;
    }
    
inline void LimitOrderBook::cancel_order(uint64_t id) {
       auto cancel = order_map.find(id); //look for the order in the hash map
       //will return an iterator

       if (cancel == order_map.end()) return; //will return an iterator pointing to one past the last element
       //indicating that its not there. 

       Order* canceled = cancel->second; //get the actual order 

        PriceLevel* level = canceled->parent_level;
        level->total_volume -= canceled->size; //cancelled order implies there is less
        //orders at that price level. 

        if (canceled->prev) {
            canceled->prev->next = canceled->next;
        } else { //implies its the head
            level->head = canceled->next;
        }
        if (canceled->next) {
            canceled->next->prev = canceled->prev;
        } else { //implies its the tail
            level->tail = canceled->prev;
        }

        if (!level->head) { //means all orders at this price level are processed
            if (level->is_bid){
                bids.erase(canceled->price);
            } 
            else {
                asks.erase(canceled->price);
            }
            
        }

        memory_pool.free(canceled); //give the data from the retired order back to our memory block!
        order_map.erase(cancel); //remove it from hash map 
    }


inline void LimitOrderBook::execute_market_sell(uint32_t size) {
        auto trades = bids.begin(); 
        
        while (trades != bids.end() && size > 0) { 

            PriceLevel& priority_bids = trades->second;
            Order* curr_bid = priority_bids.head;
            
            while (curr_bid != nullptr && size > 0) {
                if (curr_bid->size <= size) {
                    size -= curr_bid->size;
                    priority_bids.total_volume -= curr_bid->size;

                    Order* matched_bid = curr_bid;
                    curr_bid = curr_bid->next;

                    order_map.erase(matched_bid->id); //remove from tracker
                    memory_pool.free(matched_bid); //give memory of the order back to the memory pool

                } else {
                    curr_bid->size -= size; //bid stays alive
                    priority_bids.total_volume -=size;
                    size = 0; //stops the loop 
                }
            }

            if (priority_bids.total_volume == 0) { //the whole level has been cleared
                trades = bids.erase(trades);
            }
            else { //prevent dangling pointers
                if (curr_bid != nullptr) {
                    curr_bid->prev = nullptr;
                }
                priority_bids.head = curr_bid; //make sure the head is updated.
            }
        }
    }
    
inline void LimitOrderBook::execute_market_buy(uint32_t size) {
        auto trades = asks.begin(); //points to the priority asks of std::map

        while (trades != asks.end() && size > 0) { //loop continues until there is either no more ask orders (unlikely) or the user fully satisfied all of their orders
            
            PriceLevel& priority_asks = trades->second;
            Order* curr_ask = priority_asks.head;

            while (curr_ask != nullptr && size > 0) {
            if (curr_ask->size <= size) {
                size -= curr_ask->size;
                priority_asks.total_volume -= curr_ask->size;

                Order* matched_ask = curr_ask;
                curr_ask = curr_ask->next;
                order_map.erase(matched_ask->id);
                memory_pool.free(matched_ask);

            } else {
                curr_ask->size -= size;
                priority_asks.total_volume -= size;
                size = 0; 
            }

        }

            if (priority_asks.total_volume == 0) {
                trades = asks.erase(trades);
            }
            else {
                if (curr_ask != nullptr) {
                    curr_ask->prev = nullptr;
                }
                priority_asks.head = curr_ask;
            }
        }

    }