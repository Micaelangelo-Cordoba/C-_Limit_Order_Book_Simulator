# Cpp_Limit_Order_Book_Simulator

# Overview

Engineered a low-latency C++ limit order book simulator that parses millions of market orders. It processes several bid and ask orders and organizes them based on price and time priority. 

It has been engineered to process and match 5 million simulated market orders in ~1.2 seconds on consumer hardware (2019 MacBook air).

# Architecture



# Zero-Allocation Memory Pool

Dynamic memory allocation ('new'/'delete') introduces latency spikes due to kernal system calls and heap fragmentation

This engine implements a custom contiguous memory arena (free list) using a preallocated std::vector.

At startup, millions of Order Structs are allocated side-by-side in RAM. The order structs are padded and aligned (alignas(32)) to fit perfectly within 64-bit cache lines. 

During live trading, Order structs are popped from the free list memory stack in O(1) constant time, eliminating the need for requesting the Operating System for heap memory.







