#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include <set>
#include <map>
#include <random>
#include <unistd.h> // for sleep
#include <bits/stdc++.h>

using namespace std;

// global variables
int n, m, numTrans, constVal;
double lambda, readRatio;

vector<long long> commitDelay, totalAbortCnt;
vector<int> threadTrans;

// random number generator
random_device rd;
mt19937 gen(rd());

// global start time in microseconds
auto startTime = chrono::high_resolution_clock::now();
auto S = chrono::duration_cast<chrono::microseconds>(startTime.time_since_epoch()).count();

// transaction class
class transaction {
    public:
        mutex tLock; // lock for transaction
        int tID; // transaction id
        int tStatus; // transaction status: 0 - active, 1 - committed, 2 - aborted

        vector<int> localMem; // local memory of transaction

        chrono::high_resolution_clock::time_point beginT;   // begin time of transaction
        chrono::high_resolution_clock::time_point endT;     // end time of transaction

        // read and write sets
        set<int> readSet;
        set<int> writeSet;


        // constructor
        transaction(int Id) {
            tID = Id;
            tStatus = 0;
            localMem = vector<int>(m, -1);
            beginT = chrono::high_resolution_clock::now();
        }
        

        // get-set functions

        
        // read function

        // write function

        // try commit function
};

// data item class
class dataItem {
    public:
        mutex dataItemLock; // lock for data item
        int val; // value of data item
        // set<transaction*> readList; // read list


        // constructor
        dataItem(int v) {
            val = v;
            // readList = set<transaction*>();
        }

        // read function
        int read() {
            return val;
        }

        // write function
        void write(int newVal) {
            val = newVal;
        }

};

// OCC-WSI class
class OCC_WSI {
    public:
        vector<dataItem*> data; // data items
        vector<transaction*> T; // active transaction list
        vector<int> Table; // reserve table
        priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> Txs; // pending transaction queue
        int state;
        mutex OCCLock; // lock for OCC_WSI

        // constructor
        
        void executeTx(int tID, int snapshot) {
            // get read and write sets
        }

        void detectConflict(int tID, set<int> readSet, set<int> writeSet) {
            // detect conflict
            int SV = state; // snapshot version
            for (auto rec : readSet) {
                if (Table[rec] > SV) {
                    Txs.push({10, tID}); // push to pending transaction queue
                    return;
                }
            }
            OCCLock.lock();
            // update state version
            state++;
            // write set    
            // apply Tx
            for (auto rec : writeSet) {
                data[rec]->dataItemLock.lock();
            }
            for (auto rec : writeSet) {
                Table[rec] = state; 
            }
            OCCLock.unlock();
            return;
        }
    
};

// thread function



