#include <unordered_map>
#include <atomic>

using namespace std;

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

   