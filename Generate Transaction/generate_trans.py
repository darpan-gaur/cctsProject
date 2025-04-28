import random

def generate_transaction(num_dataItems):
    """Generate a single transaction with random operations"""
    num_ops = random.randint(1, 5)  
    transaction = []
    for _ in range(num_ops):
        op = random.choice(["r", "w"])
        dataItem = random.randint(1, num_dataItems)
        transaction.append((op, dataItem))
    return transaction

def generate_transactions(num_transactions, num_dataItems):
    """Generate list of transactions with random operations"""
    return [generate_transaction(num_dataItems) for _ in range(num_transactions)]

def transaction_has_conflict(t1, t2):
    """Check if two transactions have any conflicting operations"""
    for (op1, item1) in t1:
        for (op2, item2) in t2:
            if item1 == item2 and (op1 == 'w' or op2 == 'w'):
                return True
    return False

def calculate_dependency(transactions):
    """Calculate dependency percentage between transactions"""
    n = len(transactions)
    if n < 2:
        return 0.0
    
    total_pairs = n * (n - 1) // 2
    conflict_count = 0
    
    for i in range(n):
        for j in range(i+1, n):
            if transaction_has_conflict(transactions[i], transactions[j]):
                conflict_count += 1
                
    return (conflict_count / total_pairs) * 100 if total_pairs > 0 else 0.0

def save_transactions(transactions, filename="transactions.txt"):
    """Save transactions to file in required format"""
    with open(filename, 'w') as f:
        for t in transactions:
            line = ", ".join([f"{op}({acc})" for op, acc in t])
            f.write(line + '\n')

def main():
    """Main program execution with input validation"""
    while True:
        try:
            target_dep = float(input("Target dependency percentage (0-100): "))
            num_trans = int(input("Number of transactions (≥2): "))
            num_dataItems = int(input("Number of data items (≥1): "))
            
            if not (0 <= target_dep <= 100):
                print("Percentage must be 0-100")
                continue
            if num_trans < 2:
                print("Need ≥2 transactions")
                continue
            if num_dataItems < 1:
                print("Need ≥1 data items")
                continue
            break
        except ValueError:
            print("Invalid input. Numbers only.")

    max_attempts = 5
    tolerance = 1
    
    for attempt in range(1, max_attempts+1):
        transactions = generate_transactions(num_trans, num_dataItems)
        current_dep = calculate_dependency(transactions)
        
        if abs(current_dep - target_dep) <= tolerance:
            save_transactions(transactions)
            print(f"\n Success after {attempt} attempts!")
            print(f" Achieved {current_dep:.2f}% (Target: {target_dep}%)")
            print(f" Saved {len(transactions)} transactions to transactions.txt")
            return
        
        if attempt % 1 == 0:
            print(f"Attempt {attempt}: Current dependency {current_dep:.2f}%")
    
    print(f"\nFailed after {max_attempts} attempts")

if __name__ == "__main__":
    main()
