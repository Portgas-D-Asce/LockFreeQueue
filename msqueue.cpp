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
    auto producer = [&ssqs](int total, int id) {
        for(int i = 0; i < total; ++i) {
            while(!ssqs[id].push(i));
            // usleep(2);
        }
    };

    vector<int> nums(n);
    auto consumer = [&ssqs, &nums](int total) {
        size_t idx = 0, cnt = 0, m = ssqs.size();
        static size_t cur = 0;
        while(idx < total) {
            ++cnt;
            if(ssqs[cur & m - 1].empty()) continue;
            // cout << ssq.front() << endl;
            nums[ssqs[cur & m - 1].front()]++;
            ssqs[cur++ & m - 1].pop();
            ++idx;
        }
        std::cout << "N: " << total << ", " << idx << std::endl;
        std::cout << "Round: " << cnt << std::endl;
    };

    vector<thread> pts;
    for(int i = 0; i < m; ++i) {
        pts.emplace_back(producer, n, i);
    }

    thread ct(consumer, n * m);

    for(int i = 0; i < m; ++i) {
        pts[i].join();
    }
    ct.join();

    cout << "error number: " << endl;
    for(int i = 0; i < n; ++i) {
        if(nums[i] != m) {
            cout << i << endl;
        }
    }
    cout << "over" << endl;

    return 0;
}