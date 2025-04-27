# Parallel Transaction Scheduler Using DAG

## Overview
This project implements a **parallel transaction scheduler** using a **Directed Acyclic Graph (DAG)** model.  
The transactions and their dependencies (due to read/write conflicts) are processed in parallel using C++11 threads.

## File Structure
- **`DAG.h`**: Defines the `DAGmodule` class, managing the DAG structure and operations on it.
- **`transaction.h`**: Defines the `transaction` struct, containing read and write sets.
- **`parallel_scheduler_DAG.cpp`**: The main program that:
  - Reads transaction data.
  - Constructs the DAG using multiple threads.
  - Executes transactions respecting dependencies.

## Input Format
The program expects an `input.txt` file with the following content:
<number_of_threads> <number_of_transactions> <transactions_file_name>
Example:
4 10 transactions.txt

The transactions file (`transactions.txt`) should have each transaction on a new line, formatted like:
w(100), r(200), w(300) r(100), w(400) w(500) ...
Where:
- `r(address)` represents a read from `address`
- `w(address)` represents a write to `address`

## How It Works
1. **Reading Transactions**:
   - Parses the transactions file into `transaction` objects containing read and write sets.

2. **Constructing the DAG**:
   - Each transaction is added as a node.
   - Directed edges are added based on read-write, write-read, or write-write conflicts.
   - Multiple threads concurrently create the DAG.

3. **Executing Transactions**:
   - A transaction is executable when all its dependencies (incoming edges) are resolved (in-degree becomes 0).
   - Threads pick ready transactions concurrently and execute them.
   - Execution details are logged to `output_DAG.txt`.

4. **Output**:
   - The constructed DAG will be printed to the console.
   - Transaction execution logs are saved in `output_DAG.txt`.

## Compilation and Running
### CompileWhere:
- `r(address)` represents a read from `address`
- `w(address)` represents a write to `address`

## How It Works
1. **Reading Transactions**:
   - Parses the transactions file into `transaction` objects containing read and write sets.

2. **Constructing the DAG**:
   - Each transaction is added as a node.
   - Directed edges are added based on read-write, write-read, or write-write conflicts.
   - Multiple threads concurrently create the DAG.

3. **Executing Transactions**:
   - A transaction is executable when all its dependencies (incoming edges) are resolved (in-degree becomes 0).
   - Threads pick ready transactions concurrently and execute them.
   - Execution details are logged to `output_DAG.txt`.

4. **Output**:
   - The constructed DAG will be printed to the console.
   - Transaction execution logs are saved in `output_DAG.txt`.

## Compilation and Running
### Compile
```bash
g++ parallel_scheduler_DAG.cpp -o scheduler -std=c++11
./scheduler
```
