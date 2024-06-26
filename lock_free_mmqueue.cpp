#include <iostream>
#include <vector>
#include <thread>
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

static atomic<size_t> alive;
template<typename T>
class Producer {
private:
    vector<shared_ptr<SSQueue<T>>> _ssqs;
    thread _t;
private:
    void routing(size_t total) {
        size_t idx = 0, cnt = 0, n = _ssqs.size(), round = 0;
        while(cnt < total) {
            ++round;
            if(!_ssqs[++idx % n]->push(cnt)) continue;
            ++cnt;
        }
        --alive;
        printf("produce msgs %ld in %ld rounds\n", cnt, round);
    }
public:
    explicit Producer(const vector<shared_ptr<SSQueue<T>>>& ssqs) : _ssqs(ssqs) {}

    void start(size_t total) {
        _t = thread(&Producer::routing, this, total);
    }

    void join() {
        if(_t.joinable()) _t.join();
    }
};

#define CHECK

#ifdef CHECK
mutex mtx;
vector<int> nums;
void rem(int x) {
    lock_guard<mutex> lg(mtx);
    nums[x]++;
}
#endif

atomic<size_t> consume_total = 0;
template<typename T>
class Consumer {
private:
    vector<shared_ptr<SSQueue<T>>> _ssqs;
    thread _t;
private:
    void routing() {
        size_t cnt = 0, round = 0, n = _ssqs.size();
        size_t idx = 0, temp = 0;
        while(alive || temp <= 2 * n) {
            ++round, ++temp;
            if(_ssqs[++idx % n]->empty()) continue;

#ifdef CHECK
            rem(_ssqs[idx % n]->front());
#endif
            _ssqs[idx % n]->pop();
            ++cnt, temp = 0;
        }
        consume_total += cnt;
        printf("consume msgs %ld in %ld rounds\n", cnt, round);
    }
public:
    explicit Consumer(const vector<shared_ptr<SSQueue<T>>>& ssqs) : _ssqs(ssqs) {}

    void start() {
        _t = thread(&Consumer::routing, this);
    }

    void join() {
        if(_t.joinable()) _t.join();
    }
};


int main() {
    size_t m = 4, n = 4, w = 1 << 26;
    printf("%ld producers will send %ld messages totally\n", m, w);
    alive = m;
#ifdef CHECK
    nums = vector<int>(w / m);
#endif

    vector<shared_ptr<SSQueue<int>>> ssqs(m * n);
    for(int i = 0; i < m * n; ++i) {
        ssqs[i] = make_shared<SSQueue<int>>(16384);
    }

    vector<shared_ptr<Producer<int>>> ps(m);
    for(int i = 0; i < m; ++i) {
        vector<shared_ptr<SSQueue<int>>> temp(n);
        for(int j = 0; j < n; ++j) {
            temp[j] = ssqs[i * n + j];
        }
        ps[i] = make_shared<Producer<int>>(temp);
    }

    vector<shared_ptr<Consumer<int>>> cs(n);
    for(int i = 0; i < n; ++i) {
        vector<shared_ptr<SSQueue<int>>> temp(m);
        for(int j = 0; j < m; ++j) {
            temp[j] = ssqs[j * n + i];
        }
        cs[i] = make_shared<Consumer<int>>(temp);
    }

    for(int i = 0; i < m; ++i) {
        ps[i]->start(w / m);
    }

    for(int i = 0; i < n; ++i) {
        cs[i]->start();
    }

    for(int i = 0; i < m; ++i) {
        ps[i]->join();
    }

    for(int i = 0; i < n; ++i) {
        cs[i]->join();
    }

    printf("%ld consumers received %ld messages totally\n", n, consume_total.load());

#ifdef CHECK
    cout << "error number: " << endl;
    for(int i = 0; i < w / m; ++i) {
        // cout << nums[i] << endl;
        if(nums[i] != m) {
            cout << i << endl;
        }
    }
    cout << "over" << endl;
#endif

    return 0;
}