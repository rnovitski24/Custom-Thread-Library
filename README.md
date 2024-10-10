# Thread Library: A Custom User-Level Thread Management Library

**Project 3 for Operating Systems**  
Contributors: Ryan Novitski, Ross Grundy

---

## Overview
This project implements a user-level thread management library that provides basic threading functionality, including thread creation, scheduling, synchronization primitives (locks and condition variables), and context switching. The library allows for concurrent execution of user-defined functions in a simulated environment, and ensures proper thread handling, locking mechanisms, and deadlock detection.

The library is implemented using C++ with `ucontext` for managing thread contexts, and custom data structures for lock management and thread scheduling.

## Features
- **Thread Creation**: Ability to create new user-level threads and manage context switching.
- **Preemptive Scheduling**: Implements a scheduler that pulls from a ready queue and uses FIFO to ensure fair thread execution.
- **Locking Mechanisms**: Provides locks (`thread_lock` and `thread_unlock`) to control access to shared resources.
- **Condition Variables**: Implements condition variables (`thread_wait`, `thread_signal`, `thread_broadcast`) for coordinating threads.
- **Deadlock Detection**: Detects deadlock scenarios and safely exits.

## Technologies Used
- **Languages**: C++  
- **Libraries/Frameworks**: `ucontext`, standard C++ libraries  
- **Tools**: Custom interrupt handling, queue and map data structures for scheduling  

## Architecture
The thread library uses a set of custom data structures for managing threads and locks. Threads are stored in a ready queue, and lock ownership and conditional variables are tracked via unordered maps. The system ensures safe context switching and thread cleanup after execution.

Key Components:
- **Ready Queue**: FIFO structure for managing threads that are ready to run.
- **Lock Owners & Queues**: Unordered maps tracking which thread holds a lock and the queue of threads waiting for each lock.
- **Colony Tables**: Conditional variables stored in a map of lock-conditional pairs to manage waiting threads.

## Setup and Installation

### Prerequisites
- C++ compiler supporting `ucontext`
- Makefile for building the project

### Steps to Run
1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/Custom-Thread-Library.git
   cd Custom-Thread-Library
2. Compile the library:
   ```bash
   make
3. Run one of the included test programs:
   ```bash
   ./[test-program-name]