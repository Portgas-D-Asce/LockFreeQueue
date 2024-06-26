#include "lock_free_ssqueue.h"
// #define CHECK
#ifdef CHECK
vector<int> nums;
#endif


int main() {
    size_t w = 1 << 27, buffer_len = 16384 << 4;
    printf("send %ld messages with buffer length %ld\n", w, buffer_len);
#ifdef CHECK
    nums = vector<int>(w);
#endif
    SSQueue<int> ssq(buffer_len);
    auto producer = [&ssq](size_t n) {
        size_t cnt = 0, round = 0;
        while(cnt < n) {
            round++;
            if(!ssq.push(cnt)) continue;
            ++cnt;
            // 让生产者跑慢点，便于消费者追上，恶意制造竞争
            // usleep(2);
        }
        printf("produce msgs %ld in %ld rounds\n", cnt, round);
    };

    auto consumer = [&ssq](size_t n) {
        size_t cnt = 0, round = 0;
        while(cnt < n) {
            ++round;
            if(ssq.empty()) continue;
            // cout << ssq.front() << endl;
#ifdef CHECK
            ++nums[ssq.front()];
#endif
            ssq.pop();
            ++cnt;
        }
        printf("consume msgs %ld in %ld rounds\n", cnt, round);
    };
    std::thread t0(producer, w);
    std::thread t1(consumer, w);
    t0.join();
    t1.join();

#ifdef CHECK
    cout << "error number: " << endl;
    for(int i = 0; i < nums.size(); ++i) {
        if(nums[i] != 1) {
            cout << i << endl;
        }
    }
    cout << "over" << endl;
#endif

    return 0;
}