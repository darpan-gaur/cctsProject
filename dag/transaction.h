#include <set>

using namespace std;

struct transaction {
    // int txnID;
    set<int> readSet;
    set<int> writeSet;
};