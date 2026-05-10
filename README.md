# Cpp_Limit_Order_Book_Simulator

# Overview

Engineered a low-latency C++ limit order book simulator that parses millions of market orders. It processes several bid and ask orders and organizes them based on price and time priority. 

It has been engineered to process and match 5 million simulated market orders in ~1.2 seconds on consumer hardware (2019 MacBook air).

# 💻🧠 Architecture

Achieved microsecond-level latency by bypassing out-of-the-box standard data structures such as std::priority_queue.

The LOB state is managed by a fusion of a few data structures that allow for O(1) critical-path operations.

1) Price Priority: (std::map) Orders are grouped into PriceLevel buckets. The underlying red-black tree ensures that the highest-priority bids and asks are immediately accessible.
   
2) Time Priority: (doubly Linked-List) PriceLevel structs manage a doubly linked-list of Order structs. This allows for a FIFO execution of Orders and allows for an O(1) insertion of Orders.

3) Instant Cancellation: (std::unordered_map) A hash map links an Order's ID with a pointer to its physical memory address. This allows the engine to directly jump to an order, unlink it from the linked-list and free its memory back into the memory pool in O(1) time eithout traversing through the list.




# Zero-Allocation Memory Pool

Dynamic memory allocation ('new'/'delete') introduces latency spikes due to kernal system calls and heap fragmentation

This engine implements a custom contiguous memory arena (free list) using a preallocated std::vector.

At startup, millions of Order Structs are allocated side-by-side in RAM. The order structs are padded and aligned (alignas(32)) to fit perfectly within 64-bit cache lines. 

During live trading, Order structs are popped from the free list memory stack in O(1) constant time, eliminating the need for requesting the Operating System for heap memory.







