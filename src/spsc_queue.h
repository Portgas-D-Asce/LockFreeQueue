#include <atomic>
using namespace std;

template<typename T>
class SSQueue {
private:
    T* const _buff;
    const size_t _n;
    // size_t _start, _end;
    // volatile size_t _start, _end;
    volatile size_t _start, _end;
    // atomic<unsigned int> _start, _end;
public:
    explicit SSQueue(size_t n) : _buff(new T[n]), _n(n), _start(0), _end(0) {}

    SSQueue(const SSQueue& other) : _buff(new T[other._n]), _n(other._n), _start(0), _end(0) {}

    ~SSQueue() {
        delete []_buff;
    }

    bool empty() {
        return _start == _end;
    }

    bool full() {
        return  _end - _start == _n;
    }

    bool push(const T& val) {
        // 队列中元素已满（kfifo 方式可以存储到 n 个元素）
        if(_end - _start & _n) return false;

        // 保证了一旦通过索引发现队列不为空，新数据一定是可取的
        _buff[_end & _n - 1] = val;

        // 在数据完全准备好之前，绝对不能更新 _end
        // 否则，可能取走还未准备好的数据
        // atomic_thread_fence(memory_order_acquire);
        atomic_thread_fence(memory_order_release);
        ++_end;
        // _end.fetch_add(1, memory_order_release);
        return true;
    }

    void pop() {
        // 在完全取走数据之前，绝对不能更新 _start
        // 否则，新 push 进的数据可能会覆盖要取走的数据
        // atomic_thread_fence(memory_order_acquire);
        atomic_thread_fence(memory_order_release);
        ++_start;
        // _start.fetch_add(1, memory_order_release);
    }

    T& front() {
        return _buff[_start & _n - 1];
    }
};
