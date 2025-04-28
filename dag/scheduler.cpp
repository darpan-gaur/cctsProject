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

struct transaction {
    // int txnID;
    set<int> readSet;
    set<int> writeSet;
};

class DAGmodule {
    public:
        // map<int, set<int>> adjList;
        vector<vector<int>> adjList;
        atomic<int> completedTxn{0};
        atomic<int> lastTxn{0};
        atomic<int> transactionCount{0};
        unique_ptr<std::atomic<int>[]> inDegree;

        DAGmodule(int totTrans) {
            // adjList.clear();
            inDegree = unique_ptr<std::atomic<int>[]>(new std::atomic<int>[totTrans]);
            for (int i = 0; i < totTrans; ++i) {
                inDegree[i].store(0);  // Atomic store to set initial value to 0
            }
            adjList.resize(totTrans, vector<int>());
        }
        // void addNode(int txnID) {
        //     // std::cout << "A\n";
        //     inDegree[txnID].store(0);
        //     // std::cout << "B\n";
        //     adjList[txnID] = set<int>();
        //     // std::cout << "C\n";
        // }
        void addEdge(int u, int v) {
            // cout << inDegree[v].load() << " " << u << " " << v << endl;
            inDegree[v].fetch_add(1, std::memory_order_relaxed);
            
            // adjList[u].insert(v);
            adjList[u].push_back(v);
        }
        void printDAG() {
            // for (int i = 0; i < adjList.size(); i++) {
            //     cout << "Transaction " << i << ": ";
            //     for (int j = 0; j < adjList[i].size(); j++) {
            //         cout << adjList[i][j] << " ";
            //     }
            //     cout << endl;
            // }
            //print inDegree
            cout << "InDegree: ";
            for (int i = 0; i < transactionCount; i++) {
                cout << inDegree[i].load() << " ";
            }
            cout << endl;
        }
};

// global variables
int totTrans; //number of transactions

vector<transaction *> transactions;
DAGmodule *DAG;

mutex OutputMutex; // mutex for output file

// function to read transactions from the file
void readTransactions(string fileName) {
    // Each line has a transaction in the format w(369), r(356), w(819)
    ifstream fin(fileName);
    string line;
    while (getline(fin, line)) {
        // parse the line
        stringstream ss(line);
        string token;
        int txnID = transactions.size();
        transaction *t = new transaction();
        // t->txnID = txnID;
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

//m threads concurrently create DAG
void createDAG(int threadID) {
    while(true) {
        int txnID = DAG->transactionCount.fetch_add(1);
        // cout<< "Thread " << threadID << " creating DAG for transaction " << txnID << endl;
        if (txnID >= totTrans) {
            DAG->transactionCount--; 
            break; // all transactions are processed
        }
        transaction *t = transactions[txnID];
        // cout<< "Thread " << threadID << " processing transaction " << txnID << endl;
        // DAG->addNode(txnID);
        // cout<< "Thread " << threadID << " added node " << txnID << endl;
        
        for(int tid = txnID + 1; tid < totTrans; tid++) {

            bool foundDependency = false;

            transaction *t2 = transactions[tid];
            //read-write dependency
            for (int addr : t->writeSet) {
                if (t2->readSet.find(addr) != t2->readSet.end()) {
                    DAG->addEdge(txnID, tid);
                    foundDependency = true;
                    break;
                }
            }
            if(foundDependency) {
                continue;
            }
            //write-read dependency
            for (int addr : t->readSet) {
                if (t2->writeSet.find(addr) != t2->writeSet.end()) {
                    DAG->addEdge(txnID, tid);
                    foundDependency = true;
                    break;
                }
            }
            if(foundDependency) {
                continue;
            }
            //write-write dependency
            for (int addr : t->writeSet) {
                if (t2->writeSet.find(addr) != t2->writeSet.end()) {
                    DAG->addEdge(txnID, tid);
                    foundDependency = true;
                    break;
                }
            }
            
        }
        

    }
    
}

//threads concurrently search DAG for the next transaction to execute
int selectTxn(int threadID) {
    int pos = DAG->lastTxn.load();

    // First, try from pos to totTrans
    for (int i = pos; i < totTrans; i++) {
        int expected = 0;  // Fresh expected value every time
        if (DAG->inDegree[i].load() == 0) {
            if (DAG->inDegree[i].compare_exchange_strong(expected, -1)) {
                DAG->lastTxn.store(i);  // Update lastTxn
                return i;
            }
        }
    }

    // If not found, try from 0 to pos
    for (int i = 0; i < pos; i++) {
        int expected = 0;  // Fresh expected value every time
        if (DAG->inDegree[i].load() == 0) {
            if (DAG->inDegree[i].compare_exchange_strong(expected, -1)) {
                DAG->lastTxn.store(i);  // Update lastTxn
                return i;
            }
        }
    }

    // No transaction found
    return -1;
}

void executeTransaction(int txnID) {
    if(txnID < 0) {
        return;
    }
    // cout << "Executing transaction " << txnID << endl;
    for(auto it : DAG->adjList[txnID]) {
        DAG->inDegree[it].fetch_sub(1);
    }
    // cout << "Transaction " << txnID << " executed" << endl;
    DAG->completedTxn++;

    return;
}

void threadFunction(int threadID) {

    while (DAG->completedTxn < totTrans) {
        int txnID = selectTxn(threadID);
        cout<< "Thread " << threadID << " selected transaction " << txnID << endl;
        // cout<< DAG->completedTxn.load() << " transactions completed" << endl;
        if(txnID < 0) {
            OutputMutex.lock();
            ofstream fout("output_DAG.txt", ios::app);
            // fout << "Thread " << threadID << " found no transaction to execute" << endl;
            fout.close();
            OutputMutex.unlock();
        }
        else{
            executeTransaction(txnID);

            OutputMutex.lock();
            ofstream fout("output_DAG.txt", ios::app);
            fout << "Thread " << threadID << " executing transaction " << txnID << endl;
            fout << "Read set: ";
            for (int addr : transactions[txnID]->readSet) {
                fout << addr << " ";
            }
            fout << endl;
            fout << "Write set: ";
            for (int addr : transactions[txnID]->writeSet) {
                fout << addr << " ";
            }
            fout << endl;
            fout << "Transaction " << txnID << " completed" << endl;
            fout.close();
            OutputMutex.unlock();

        }
    
        
    }
    return;
}

int main() {
    string TransactionsFile;
    int n; //n = number of threads
    
    //Read input from file
    ifstream fin("input.txt");
    fin >> n >> totTrans  >>  TransactionsFile;
    cout << "Number of threads: " << n << endl;
    cout << "Number of transactions: " << totTrans << endl;
    cout << "Transactions file: " << TransactionsFile << endl; 
    fin.close();

    //initialize DAG
    DAG = new DAGmodule(totTrans);

    cout<< "Reading transactions from file..." << endl;
    //read transactions from the file
    readTransactions(TransactionsFile);
    cout<< "Transactions read successfully" << endl;

    //create DAG using n threads
    cout<<" Created DAG using " << n << " threads" << endl;
    vector<thread> threads(n);
    for (int i = 0; i < n; i++) {
        threads[i] = thread(createDAG, i);
    }
    
    
    // Join all threads
    for (int i = 0; i < n; i++) {
        threads[i].join();
    }
    //output the DAG
    DAG->printDAG();
    cout<< "Printing DAG completed" << endl;

    //create threads to execute transactions
    vector<thread> execThreads(n);
    for (int i = 0; i < n; i++) {
        execThreads[i] = thread(threadFunction, i);
    }

    // Join all threads 
    for (int i = 0; i < n; i++) {
        execThreads[i].join();
    }
    cout << "All transactions executed" << endl;


    delete DAG;
    for (auto t : transactions) {
        delete t;
    }
    return 0;
}
