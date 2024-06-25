#include <iostream>
#include <vector>
#include <thread>
#include <unistd.h>
#include <algorithm>
#include <mutex>
using namespace std;

template<typename T>
class SSQueue {
private:
    T* const _buff;
    const size_t _n;
    size_t _start, _end;
    mutable mutex _mtx;
public:
    explicit SSQueue(size_t n) : _buff(new T[n]), _n(n), _start(0), _end(0) {}

    ~SSQueue() {
        delete []_buff;
    }

    bool empty() {
        lock_guard<mutex> lg(_mtx);
        return _start == _end;
    }

    bool push(const T& val) {
        lock_guard<mutex> lg(_mtx);
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

SSQueue<int> ssq(16384);
// vector<int> nums;
int N = 1 << 25;
void producer() {
    for(int i = 0; i < N; ++i) {
        while(!ssq.push(i));
        // 让生产者跑慢点，便于消费者追上，恶意制造竞争
        // usleep(2);
    }
}

void consumer() {
    size_t idx = 0, cnt = 0;
    while(idx < N) {
        ++cnt;
        if(ssq.empty()) continue;
        // cout << ssq.front() << endl;
        // nums.push_back(ssq.front());
        ssq.pop();
        ++idx;
    }
    std::cout << "N: " << N << ", " << idx << std::endl;
    std::cout << "Round: " << cnt << std::endl;
}

int main() {
    std::thread t0(producer);
    std::thread t1(consumer);
    t0.join();
    t1.join();
    // cout << "sorted: " << is_sorted(nums.begin(), nums.end()) << endl;
    // for(int i = 1; i < nums.size(); ++i) {
    //     if(nums[i] - nums[i - 1] != 1) {
    //         cout << nums[i - 1] << " vs " << nums[i] << ": "
    //              << nums[i] - nums[i - 1] << endl;
    //     }
    // }

    return 0;
}