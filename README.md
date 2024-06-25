# Concurrency


# mutex_ssqueue
互斥锁版单生产者单消费者消息队列，ops/s 在百万级别
```bash
time ./mutex_ssqueue 
N: 33554432, 33554432
Round: 34077260
./mutex_ssqueue  15.99s user 14.79s system 162% cpu 18.940 total
```

# ssqueue
无锁版单生产者单消费者，ops/s 在千万级别
```bash
time ./ssqueue 
N: 33554432, 33554432
Round: 104362965
./ssqueue  3.98s user 0.00s system 199% cpu 1.996 total
```

实现时注意两点：
- push 操作：数据没有完全加入队列，绝对不能更新 “写下标”
- pop 操作：数据没有完全取出，绝对不能更新 “读下标”

# mutex_mmqueue
互斥锁版多生产者多消费者，ops/s 单线程百万级别、多线程协同千万级别
```bash
time ./mutex_mmqueue
N: 16777216, 16777216
Round: 18356183
N: 16777216, 16777216
Round: 18330027
N: 16777216, 16777216
Round: 18126800
N: 16777216, 16777216
Round: 18074659
./mutex_mmqueue  4.91s user 21.76s system 399% cpu 6.668 total
```

# mmqueue
无锁版多生产者多消费者。额，这效率...... 主要是由于当前实现有两个致命缺陷

无锁快是毋庸置疑的，ssqueue 中可是千万级 ops/s 的，如果能在多线程中保留!!!

```bash
time ./mmqueue
N: 16777216, 16777216
Round: 13792682915
N: 16777216, 16777216
Round: 13807136029
N: 16777216, 16777216
Round: 13838903636
N: 16777216, 16777216
Round: 13855190886
./mmqueue  286.05s user 80.92s system 679% cpu 53.991 total
```