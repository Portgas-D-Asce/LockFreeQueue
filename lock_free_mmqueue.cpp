#include "lock_free_ssqueue.h"


static atomic<size_t> alive;
template<typename T>
class Producer {
private:
    // 一个生产者拥有 n (消费者数量) 个队列
    vector<shared_ptr<SSQueue<T>>> _ssqs;
    thread _t;
private:
    // 向多个消息队列中总共发送 total 个消息
    void routing(size_t total) {
        size_t idx = 0, cnt = 0, n = _ssqs.size(), round = 0;
        while(cnt < total) {
            ++round;
            // 整体轮流策略，先第 0 个队列，再第 1 个队列......
            // 发送失败（队列满了），不会在同一个队列上停留
            if(!_ssqs[++idx & n - 1]->push(cnt)) continue;
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

// #define CHECK

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
    // 一个消费者拥有 m (生产者数量) 个队列
    vector<shared_ptr<SSQueue<T>>> _ssqs;
    thread _t;
private:
    void routing() {
        size_t cnt = 0, round = 0, n = _ssqs.size(), idx = 0, temp = 0;
        // 直到 “没有生产者存在” 且 “所有队列都没有消息了” 则停止接收
        // 不保证接收完所有数据
        while(alive || temp <= n) {
            ++round;
            ++temp;
            // 当接收失败（队列为空）时，不会在某个队列上停留
            if(_ssqs[++idx & n - 1]->empty()) continue;

#ifdef CHECK
            rem(_ssqs[idx % n]->front());
#endif
            _ssqs[idx & n - 1]->pop();
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
    size_t m = 4, n = 4, w = 1 << 28;
    printf("%ld producers will send %ld messages totally\n", m, w);
    alive = m;
#ifdef CHECK
    nums = vector<int>(w / m);
#endif

    // 创建 m * n 个队列
    vector<shared_ptr<SSQueue<int>>> ssqs(m * n);
    for(int i = 0; i < m * n; ++i) {
        ssqs[i] = make_shared<SSQueue<int>>(16384 << 8);
    }

    // 创建 m 个生产者
    vector<shared_ptr<Producer<int>>> ps(m);
    for(int i = 0; i < m; ++i) {
        // 第 0 个生产者拥有第 0、1、2、3 号队列
        // 第 1 个生产者拥有第 4、5、6、7 号队列
        vector<shared_ptr<SSQueue<int>>> temp(n);
        for(int j = 0; j < n; ++j) {
            temp[j] = ssqs[i * n + j];
        }
        ps[i] = make_shared<Producer<int>>(temp);
    }

    vector<shared_ptr<Consumer<int>>> cs(n);
    for(int i = 0; i < n; ++i) {
        // 第 0 个消费者拥有第 0、4、8、12 号队列
        // 第 1 个消费者拥有第 1、5、9、13 号队列
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