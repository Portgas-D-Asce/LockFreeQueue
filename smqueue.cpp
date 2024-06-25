#include <iostream>
#include <vector>
#include <thread>
#include <unistd.h>
#include <algorithm>
#include <atomic>
#include <vector>
using namespace std;

template<typename T>
class SSQueue {
private:
    T* const _buff;
    const size_t _n;
    size_t _start;
    atomic<size_t> _end;
public:
    explicit SSQueue(size_t n) : _buff(new T[n]), _n(n), _start(0), _end(0) {}

    SSQueue(const SSQueue& other) : _buff(new T[other._n]), _n(other._n), _start(0), _end(0) {}

    ~SSQueue() {
        delete []_buff;
    }

    bool empty() const {
        return _start == _end;
    }

    bool push(const T& val) {
        if(_end - _start == _n) return false;
        _buff[_end & _n - 1] = val;
        ++_end;
        return true;
    }

    void pop() {
        ++_start;
    }

    T& front() {
        return _buff[_start & _n - 1];
    }
};


int main() {
    int m = 4, n = 1 << 24;

    vector<SSQueue<int>> ssqs;
    for(int i = 0; i < m; ++i) {
        ssqs.emplace_back(16384);
    }
    auto producer = [&ssqs](int total) {
        size_t m = ssqs.size(), cur = 0;
        for(int i = 0; i < total; ++i) {
            while(!ssqs[cur & m - 1].push(i));
            ++cur;
            // usleep(2);
        }
    };

    vector<int> nums(m * n);
    auto consumer = [&ssqs, &nums](int total, int id) {
        size_t idx = 0, cnt = 0;
        // static mutex mtx;
        while(idx < total) {
            ++cnt;
            if(ssqs[id].empty()) continue;
            // cout << ssq.front() << endl;
            {
                // lock_guard<mutex> lg(mtx);
                // nums[ssqs[id].front()]++;
            }

            ssqs[id].pop();
            ++idx;
        }
        std::cout << "N: " << total << ", " << idx << std::endl;
        std::cout << "Round: " << cnt << std::endl;
    };

    vector<thread> cts;
    for(int i = 0; i < m; ++i) {
        cts.emplace_back(consumer, n, i);
    }

    thread pt(producer, n * m);

    for(int i = 0; i < m; ++i) {
        cts[i].join();
    }
    pt.join();

    // cout << "error number: " << endl;
    // for(int i = 0; i < n; ++i) {
    //     if(nums[i] != 1) {
    //         cout << i << endl;
    //     }
    // }
    // cout << "over" << endl;

    return 0;
}