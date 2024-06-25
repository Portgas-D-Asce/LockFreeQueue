#include <iostream>
#include <vector>
#include <thread>
#include <unistd.h>
#include <algorithm>
#include <atomic>
#include <mutex>
using namespace std;

template<typename T>
class MMQueue {
private:
    T* const _buff;
    const size_t _n;
    // _start 数据起始位置，也就是读端
    // _center 已就绪数据的尾部，它到 _end 之间的数据还未准备好，不允许访问
    // _end 数据的末尾，也就是写端
    atomic<size_t> _start, _center, _end;
public:
    explicit MMQueue(size_t n) : _buff(new T[n]), _n(n), _start(0), _center(0), _end(0) {}

    ~MMQueue() {
        delete []_buff;
    }

    bool push(const T& val) {
        // 抢写位置
        size_t pos = _end;
        do {
            // 所有写位置都被抢光了
            if(_end - _start == _n) return false;
            // 参与抢的过程了，但是很不幸没抢到，必须重新抢
        } while(!_end.compare_exchange_strong(pos, pos + 1));

        // 成功抢到写位置，将数据放入抢到位置上
        _buff[pos & _n - 1] = val;

        // 你这个位置数据是准备好了，允许别人读了，但是前面位置呢，可能还没有准备好，不能跨过它来读你
        // 相当于前面的人加了一把锁，锁没到达，当前位置是无法访问的，即使数据已经准备好了
        while(_center != pos) this_thread::yield();

        // 前面所有位置数据都准备好了，而且我早都准备好了，就差将位置标记为可读了，现在终于可以标记了
        _center++;
        return true;
    }

    bool pop(T& val) {
        /*
         * ❌错解
         *
         * 第一点：假设一开始队列为空，val 取到了一个无效数据，然后 _center 更新，队列不为空
         * 检查 _start 是否变化，没有，更新 _start 并退出循环
         * 返回 true 和 无效数据
         *
         * 第二点：开始 _start + 1 == _center, 循环时时发现 _start 已发生变化 _start == _center
         * 将 _start 赋值给 pos, 循环内部取值, 但这是一个无效数据啊。。。。
         * _center 自增后且 _start 未发生变化，跳出循环，返回 true 和 无效数据
         */
        /*bool flag = true;
        size_t pos = _start;
        val = _buff[pos & _n - 1];
        while((flag = (_start != _center)) && !_start.compare_exchange_strong(pos, pos + 1)) {
            val = _buff[pos & _n - 1];
        }
        return flag;*/

        /* ✅正解
         *
         * 检查的时候队列不为空，赋值的时候 _start 变化了，队列为空，val 取到了无效值
         * 但无效值一定是因为 _start 变化引起的，CAS 势必不会检查通过，也就不会返回无效值
         * 关键点：val 取值必须放在队列判空后面，CAS 前面
         * */
        size_t pos = _start;
        do {
            if(pos == _center) return false;
            val = _buff[pos & _n - 1];
        } while (!_start.compare_exchange_strong(pos, pos + 1));

        return true;
    }

};


int main() {
    MMQueue<int> mmq(65536);
    int n = 1 << 20;
    static vector<int> st(n);

    auto producer = [&mmq](int total) {
        for(int i = 0; i < total; ++i) {
            while(!mmq.push(i));
            usleep(2);
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
                lock_guard<mutex> lg(mtx);
                st[x]++;
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

    int m2 = 4;
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

    cout << "error number: " << endl;
    for(int i = 0; i < n; ++i) {
        if(st[i] != m1) {
            cout << i << endl;
        }
    }
    cout << "over" << endl;

    return 0;
}
