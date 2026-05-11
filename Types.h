#pragma once
#ifndef TYPES_H
#define TYPES_H
#include <cstdint>



enum class Type : char { //will be used by simulated network packets to indicate what type of order they are
    ADD_BUY = 'B', 
    ADD_SELL = 'S',
    CANCEL = 'C',
    MKT_BUY = 'N',
    MKT_SELL = 'M'
};

struct NetworkPacket { //represents a fully parsed packet that needs to be processed by the order book
   
    uint64_t n_id;
    uint64_t n_price;
    uint64_t n_timestamp;
    uint32_t n_size;
    Type N_TYPE;
};

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



#endif