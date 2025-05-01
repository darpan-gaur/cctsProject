#include <mutex>

using namespace std;

class dataItem {
    public:
        mutex dataItemLock; // lock for data item
        int val; // value of data item

        // constructor
        dataItem(int v) {
            val = v;
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