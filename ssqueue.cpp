#include <iostream>
#include <vector>
#include <thread>
#include <unistd.h>
#include <algorithm>
#include <atomic>
using namespace std;

#define barrier() __asm__ __volatile__("": : :"memory")

template<typename T>
class SSQueue {
private:
    T* _buff;
    size_t _n;
    // size_t _start, _end;
    // volatile size_t _start, _end;
    atomic<size_t> _start, _end;
public:
    explicit SSQueue(size_t n) : _n(n), _start(0), _end(0) {
        _buff = new T[_n];
    }

    ~SSQueue() {
        delete []_buff;
        _buff = nullptr;
    }

    bool empty() {
        return _start == _end;
    }

    // ring buffer 存在空间限制，故定义返回 bool
    bool push(const T& val) {
        // 队列中元素已满（kfifo 方式可以存储到 n 个元素）
        if(_end - _start == _n) return false;
        // 先赋值, 后更新索引值！！！
        // 保证了一旦通过索引发现队列不为空，新数据一定是可取的
        _buff[_end & _n - 1] = val;
        // barrier();
        _end++;
        return true;
    }

    void pop() {
        ++_start;
    }

    T& front() {
        return _buff[_start & _n - 1];
    }
};

SSQueue<int> ssq(65536);
vector<int> nums;
int N = 1 << 20;
void producer() {
    for(int i = 0; i < N; ++i) {
        while(!ssq.push(i));
        // 让生产者跑慢点，便于消费者追上，恶意制造竞争
        usleep(2);
    }
}

void consumer() {
    size_t idx = 0, cnt = 0;
    while(idx < N) {
        ++cnt;
        if(ssq.empty()) continue;
        cout << ssq.front() << endl;
        nums.push_back(ssq.front());
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
    cout << "sorted: " << is_sorted(nums.begin(), nums.end()) << endl;
    for(int i = 1; i < nums.size(); ++i) {
        if(nums[i] - nums[i - 1] != 1) {
            cout << nums[i - 1] << " vs " << nums[i] << ": "
                 << nums[i] - nums[i - 1] << endl;
        }
    }

    return 0;
}