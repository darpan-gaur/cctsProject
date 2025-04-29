#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include <set>
#include <unordered_map>
#include <map>
#include <random>
#include <unistd.h> // for sleep
#include <atomic>
#include <fstream>
#include <sstream>

using namespace std;

class transaction {
public:
    int tID; // transaction id
    int snapVer;
    set<int> readSet;
    set<int> writeSet;
};

vector<transaction*> transactions;
int totTrans = 0;

mutex OutputMutex;

// open output file
FILE *logFile = fopen("output_OCC.txt", "w");

void readTransactions(string fileName) {
    // Each line has a transaction in the format w(369), r(356), w(819)
    ifstream fin(fileName);
    string line;
    while (getline(fin, line)) {
        // parse the line
        stringstream ss(line);
        string token;
        int txnID = transactions.size();
        transaction* t = new transaction();
        t->tID = txnID;
        while (getline(ss, token, ',')) {
            token.erase(0, token.find_first_not_of(' '));
            if (token[0] == 'r') {
                int addr = stoi(token.substr(2, token.length() - 3));
                t->readSet.insert(addr);
            } else if (token[0] == 'w') {
                int addr = stoi(token.substr(2, token.length() - 3));
                t->writeSet.insert(addr);
            }
        }
        // add transaction to the list
        transactions.push_back(t);
    }
    fin.close();
}

class OCC_WSI {
public:
    // vector<int> Table; // reserve table
    int state;
    atomic<int> completedTxn{0};
    atomic<int> lastTxn{0};
    atomic<int> transactionCount{0};
    unique_ptr<std::atomic<int>[]> status;
    unique_ptr<std::atomic<int>[]> Table;

    OCC_WSI(int size, int state) {
        // Table.resize(size, 0);
        Table = unique_ptr<std::atomic<int>[]>(new std::atomic<int>[size]);
        this->state = state;
        status = unique_ptr<std::atomic<int>[]>(new std::atomic<int>[size]);
        for (int i = 0; i < size; ++i) {
            status[i].store(0);  // Atomic store to set initial value to 0
            Table[i].store(0);  // Atomic store to set initial value to 0
        }
    }

    int selectTxn() {
        int pos = lastTxn.load();
        // First, try from pos to totTrans
        for (int i = pos; i < totTrans; i++) {
            int expected = 0;  // Fresh expected value every time
            if (status[i].load() == 0) {
                if (status[i].compare_exchange_strong(expected, -1)) {
                    
                    lastTxn.store(i);  // Update lastTxn
                    return i;
                }
            }
        }
        // If not found, try from 0 to pos
        for (int i = 0; i < pos; i++) {
            int expected = 0;  // Fresh expected value every time
            if (status[i].load() == 0) {
                if (status[i].compare_exchange_strong(expected, -1)) {
                    
                    lastTxn.store(i);  // Update lastTxn
                    return i;
                }
            }
        }
        // No transaction found
        return -1;
    }

    void executeTx(int txnID, int threadID) {
        // get read and write sets
        status[txnID].store(1);
        cout << "Thread " << threadID << " selected transaction " << txnID << endl;
    }
    int detectConflict(int tID, set<int> readSet, set<int> writeSet) {
        transaction *t = transactions[tID];
        for (int rec: readSet) {
            if (Table[rec].load() > t->snapVer) {
                // cout << "Transaction " << tID << " aborted due to read-write conflict with transaction " << Table[rec].load() << endl;
                status[tID].store(0);
                return -1;
            }
        }

        state = t->snapVer + 1;
        for (int rec: writeSet) {
            Table[rec].store(state);
        }

        return 0;
    }
};

OCC_WSI *OCC;

void threadFunction(int threadID) {
    while (OCC->completedTxn < totTrans) {
        int txnID = OCC->selectTxn();
        
        if (txnID < 0) {
            continue;
        }
        else {
            OCC->executeTx(txnID, threadID);
            transaction *t = transactions[txnID];
            t->snapVer = OCC->state;
            int conflictResult = OCC->detectConflict(txnID, t->readSet, t->writeSet);
            if (conflictResult == -1) {
                // Transaction aborted
                OutputMutex.lock();
                // ofstream fout("output_OCC.txt", ios::app);
                // fout << "Thread " << threadID << " aborted transaction " << txnID << endl;
                // fout.close();
                fprintf(logFile, "Thread %d aborted transaction %d\n", threadID, txnID);
                OutputMutex.unlock();
                continue;
            }
            OCC->completedTxn++;

            OutputMutex.lock();
            fprintf(logFile, "Thread %d executing transaction %d\n", threadID, txnID);
            // fout << "Read set: ";
            // for (int addr : t->readSet) {
            //     fout << addr << " ";
            // }
            // fout << endl;
            // fout << "Write set: ";
            // for (int addr : t->writeSet) {
            //     fout << addr << " ";
            // }
            // fout << endl;
            fprintf(logFile, "Transaction %d completed\n", txnID);
            OutputMutex.unlock();
        }
    }
}

int main() {
    string TransactionsFile;
    int n; //n = number of threads

    //Read input from file
    ifstream fin("input.txt");
    fin >> n >> totTrans >> TransactionsFile;
    cout << "Number of threads: " << n << endl;
    cout << "Number of transactions: " << totTrans << endl;
    cout << "Transactions file: " << TransactionsFile << endl;
    fin.close();

    //initialize OCC_WSI
    OCC = new OCC_WSI(totTrans, 0);
    cout << "Reading transactions from file..." << endl;
    readTransactions(TransactionsFile);
    cout << "Transactions read successfully." << endl;

    // create threads
    vector<thread> threads;
    for (int i = 0; i < n; i++) {
        threads.push_back(thread(threadFunction, i));
    }
    // join threads
    for (int i = 0; i < n; i++) {
        threads[i].join();
    }
    cout << "All transactions completed." << endl;

    // close output file
    fclose(logFile);

    // Clean up
    for (auto t : transactions) {
        delete t;
    }
    delete OCC;


    return 0;
}