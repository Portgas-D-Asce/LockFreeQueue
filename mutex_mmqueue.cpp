#include <iostream>
#include <vector>
#include <thread>
#include <unistd.h>
#include <algorithm>
#include <mutex>
using namespace std;

template<typename T>
class MMQueue {
private:
    T* const _buff;
    const size_t _n;
    size_t _start, _end;
    mutable mutex _mtx;
public:
    explicit MMQueue(size_t n) : _buff(new T[n]), _n(n), _start(0), _end(0) {}

    ~MMQueue() {
        delete []_buff;
    }

    bool push(const T& val) {
        lock_guard<mutex> lg(_mtx);
        if(_end - _start == _n) return false;
        _buff[_end & _n - 1] = val;
        ++_end;
        return true;
    }

    bool pop(T& val) {
        lock_guard<mutex> lg(_mtx);
        if(_start == _end) return false;
        val = _buff[_start & _n - 1];
        ++_start;
        return true;
    }

};


int main() {
    MMQueue<int> mmq(65536);
    int n = 1 << 24;
    // static vector<int> st(n);

    auto producer = [&mmq](int total) {
        for(int i = 0; i < total; ++i) {
            while(!mmq.push(i));
            // usleep(1);
        }
    };

    auto consumer = [&mmq](int total) {
        static mutex mtx;
        size_t idx = 0, cnt = 0;
        while(idx < total) {
            ++cnt;
            int x = 0;
            if(!mmq.pop(x)) continue;
            // cout << x << endl;
            {
                // lock_guard<mutex> lg(mtx);
                // st[x]++;
                ++idx;
            }

        }
        std::cout << "N: " << total << ", " << idx << std::endl;
        std::cout << "Round: " << cnt << std::endl;
    };

    int m1 = 4;
    vector<thread> pts;
    for(int i = 0; i < m1; ++i) {
        pts.emplace_back(producer, n);
    }

    int m2 = 1;
    vector<thread> cts;
    for(int i = 0; i < m2; ++i) {
        cts.emplace_back(consumer, n * m1 / m2);
    }

    for(int i = 0; i < m1; ++i) {
        pts[i].join();
    }
    for(int i = 0; i < m2; ++i) {
        cts[i].join();
    }

    // cout << "error number: " << endl;
    // for(int i = 0; i < n; ++i) {
    //     if(st[i] != m1) {
    //         cout << i << endl;
    //     }
    // }
    // cout << "over" << endl;

    return 0;
}
