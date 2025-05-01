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
#include <unistd.h> // for sleep

#include "dataItem.h"
#include "transaction.h"

using namespace std;

int numDataItems = 1005; // number of data items
int totTrans = 0;
double lambda = 20;

vector<transaction*> transactions;
vector<long long> totalAbortCnt;

// random number generator
random_device rd;
mt19937 gen(rd());
// global start time in microseconds
auto startTime = chrono::high_resolution_clock::now();
auto S = chrono::duration_cast<chrono::microseconds>(startTime.time_since_epoch()).count();

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
        t->localMem = vector<int>();
        for (int i = 0; i < numDataItems; i++) {
            t->localMem.push_back(-1);
        }
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
    vector<dataItem*> dataItems;
    

    OCC_WSI(int size, int state) {
        // Table.resize(size, 0);
        Table = unique_ptr<std::atomic<int>[]>(new std::atomic<int>[size]);
        this->state = state;
        status = unique_ptr<std::atomic<int>[]>(new std::atomic<int>[size]);
        for (int i = 0; i < size; ++i) {
            status[i].store(0);  // Atomic store to set initial value to 0
            Table[i].store(0);  // Atomic store to set initial value to 0
        }
        dataItems = vector<dataItem*>();
        for (int i = 0; i < numDataItems; i++) {
            dataItems.push_back(new dataItem(0));
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
        // exponential_distribution<double> exp(lambda);
        // sleep(exp(gen)); // Simulate some work

        transaction *t = transactions[txnID];
        for (int rec: t->readSet) {
            dataItems[rec]->dataItemLock.lock();
            t->localMem[rec] = dataItems[rec]->read();
            // cout << "Thread " << threadID << " read from " << rec << " value " << t->localMem[rec] << endl;
            dataItems[rec]->dataItemLock.unlock();
        }

        for (int rec: t->writeSet) {
            // get random value
            int randVal = rand() % 10;
            t->localMem[rec] += randVal;
        }


        // cout << "Thread " << threadID << " selected transaction " << txnID << endl;
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
            dataItems[rec]->dataItemLock.lock();
            dataItems[rec]->write(t->localMem[rec]);
            dataItems[rec]->dataItemLock.unlock();
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
                totalAbortCnt[threadID]++;
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
    // cout << "Number of threads: " << n << endl;
    // cout << "Number of transactions: " << totTrans << endl;
    // cout << "Transactions file: " << TransactionsFile << endl;
    fin.close();

    totalAbortCnt.resize(n, 0);

    //initialize OCC_WSI
    OCC = new OCC_WSI(totTrans, 0);
    // cout << "Reading transactions from file..." << endl;
    readTransactions(TransactionsFile);
    // cout << "Transactions read successfully." << endl;

    // start time 
    auto thrdStartTime = chrono::high_resolution_clock::now();
    auto tST = chrono::duration_cast<chrono::microseconds>(thrdStartTime.time_since_epoch()).count();

    // create threads
    vector<thread> threads;
    for (int i = 0; i < n; i++) {
        threads.push_back(thread(threadFunction, i));
    }
    // join threads
    for (int i = 0; i < n; i++) {
        threads[i].join();
    }
    // cout << "All transactions completed." << endl;


    auto thrdEndTime = chrono::high_resolution_clock::now();
    auto elapsedTime = chrono::duration_cast<chrono::microseconds>(thrdEndTime - thrdStartTime).count();
    // cout << "Elapsed time: " << elapsedTime << " microseconds" << endl;
    cout << elapsedTime << " ";
    long long totalAborts=0;
    for (int i = 0; i < n; i++) {
        totalAborts += totalAbortCnt[i];
    }
    // cout << "Total aborts: " << totalAborts << endl;
    cout << totalAborts << endl;

    // close output file
    fclose(logFile);

    // Clean up
    for (auto t : transactions) {
        delete t;
    }
    for (auto di : OCC->dataItems) {
        delete di;
    }
    delete OCC;


    return 0;
}