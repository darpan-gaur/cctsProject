# Transaction Dependency Generator

This Python program generates random transactions, calculates the dependency (conflict) percentage between them, and saves a set of transactions matching a user-specified dependency percentage to a file.

---

## Features

- **Random Transaction Generation:**  
  Each transaction contains a random number of read (`r`) and write (`w`) operations on randomly chosen data items.

- **Customizable Parameters:**  
  User specifies the target dependency percentage, number of transactions, and number of data items.

- **Accurate Dependency Calculation:**  
  The program detects conflicts (dependencies) between transactions based on read-write and write-write access to the same data item.

- **Automated Search:**  
  Repeatedly generates transaction sets until the desired dependency percentage is achieved (within a small tolerance).

- **Output:**  
  Saves the successful set of transactions to `transactions.txt` in a clear, readable format.

---

## How It Works

1. **Input:**  
   The user is prompted to enter:
   - Target dependency percentage (0–100)
   - Number of transactions (≥2)
   - Number of data items (≥1)

2. **Transaction Generation:**  
   Each transaction is a sequence of 1–4 random operations, where each operation is:
   - `r(X)` — read from data item X
   - `w(X)` — write to item X

3. **Dependency Calculation:**  
   - For every pair of transactions, a conflict is counted if both access the same data item and at least one is a write.
   - Dependency percentage = (number of conflicting pairs / total pairs) × 100

4. **Output:**  
   - When a set of transactions matches the target dependency percentage (within 0.01%), it is saved to `transactions.txt`.
   - Each line in the file represents a transaction, with operations separated by commas, e.g.:
     ```
     r(2), w(1), r(3)
     w(2), r(1)
     r(3)
     ```

---

