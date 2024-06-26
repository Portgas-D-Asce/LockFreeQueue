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
    T* const _buff;
    const size_t _n;
    // size_t _start, _end;
    // volatile size_t _start, _end;
    // atomic<size_t> _start, _end;
    size_t _start;
    atomic<size_t> _end;
public:
    explicit SSQueue(size_t n) : _buff(new T[n]), _n(n), _start(0), _end(0) {}

    SSQueue(const SSQueue& other) : _buff(new T[other._n]), _n(other._n), _start(0), _end(0) {}

    ~SSQueue() {
        delete []_buff;
    }

    bool empty() {
        return _start == _end.load(memory_order_relaxed);
    }

    // ring buffer 存在空间限制，故定义返回 bool
    bool push(const T& val) {
        // 队列中元素已满（kfifo 方式可以存储到 n 个元素）
        if(_end - _start & _n) return false;
        // 先赋值, 后更新索引值！！！
        // 保证了一旦通过索引发现队列不为空，新数据一定是可取的
        _buff[_end & _n - 1] = val;
        // barrier();
        // _end.fetch_add(1, memory_order_release);
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
