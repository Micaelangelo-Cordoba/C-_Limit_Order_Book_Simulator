# Cpp_Limit_Order_Book_Simulator

# Overview

Engineered a low-latency limit order book simulator (LOB) written in modern C++.

This simulator processes millions of simulated orders, organizing bids and asks in price-time priority. 

Through the utilization of custom memory management and efficient data structures, the engine achieves deterministic performance and processes over **3.3 million orders per second** on consumer hardware. 

# Performance Metrics 🚀
**Throughput:** ~3.3 million orders per second.

**Average latency:** 300 nanoseconds per order

**Hardware Profile:** Tested on a 2019 Intel-based MacBook Air.


# 💻🧠 Architecture

Achieved microsecond-level latency by bypassing out-of-the-box standard data structures such as std::priority_queue.

The LOB state is managed by a fusion of a few data structures that allow for O(1) critical-path operations.

1) **Price Priority: (std::map)** Orders are grouped into PriceLevel buckets. The underlying red-black tree ensures that the highest-priority bids and asks are immediately accessible.
   
2) **Time Priority: (doubly Linked-List)** PriceLevel structs manage a doubly linked-list of Order structs. This allows for a FIFO execution of Orders and allows for an O(1) insertion of Orders.

3) **Instant Cancellation: (std::unordered_map)** A hash map links an Order's ID with a pointer to its physical memory address. This allows the engine to directly jump to an order, unlink it from the linked-list and free its memory back into the memory pool in O(1) time without traversing through the list.




# Custom Free-List Memory Pool

Dynamic memory allocation ('new'/'delete') introduces latency spikes due to kernel system calls and heap fragmentation

This engine implements a custom contiguous memory arena (free list) using a preallocated std::vector.

At startup, millions of Order Structs are allocated side-by-side in RAM. The order structs are padded and aligned (alignas(32)) to fit perfectly within 64-byte cache lines. 

During live trading, Order structs are popped from the free list memory stack in O(1) constant time, minimizing the need for requesting the Operating System for heap memory.

# How To Run (Linux/MacOS)

1) **Clone The Repository**
   
    git clone https://github.com/Micaelangelo-Cordoba/Cpp_Limit_Order_Book_Simulator.git

3) **Move to Main Directory**
   
    cd Cpp_Limit_Order_Book_Simulator

5) **run executable**
   
    ./matching_engine







