#include <unordered_map>
#include <atomic>

using namespace std;

class DAGmodule {
    public:
        unordered_map<int, set<int>> adjList;
        atomic<int> completedTxn{0};
        atomic<int> lastTxn{0};
        atomic<int> transactionCount{0};
        unique_ptr<std::atomic<int>[]> inDegree;

        DAGmodule(int totTrans) {
            adjList.clear();
            inDegree = unique_ptr<std::atomic<int>[]>(new std::atomic<int>[totTrans]);
        }
        void addNode(int txnID) {
            // std::cout << "A\n";
            inDegree[txnID].store(0);
            // std::cout << "B\n";
            adjList[txnID] = set<int>();
            // std::cout << "C\n";
        }
        void addEdge(int u, int v) {
            inDegree[v].fetch_add(1);
            adjList[u].insert(v);
        }
        void printDAG() {
            for (auto it : adjList) {
                cout << "Transaction " << it.first << ": ";
                for (auto it2 : it.second) {
                    cout << it2 << " ";
                }
                cout << endl;
            }
            //print inDegree
            cout << "InDegree: ";
            for (int i = 0; i < transactionCount; i++) {
                cout << inDegree[i].load() << " ";
            }
            cout << endl;
        }
};

   