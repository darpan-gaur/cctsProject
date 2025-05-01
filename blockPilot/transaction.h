#include <vector>
#include <set>

using namespace std;

class transaction {
    public:
        int tID; // transaction id
        int snapVer;
        vector<int> localMem; // local memory of transaction
        set<int> readSet;
        set<int> writeSet;
    
};