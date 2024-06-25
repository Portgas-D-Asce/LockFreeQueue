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

# msqueue

无锁多生产者，单消费者，瓶颈在单消费者

可以将其拆分为多个 单生产者 + 单消费者模型
- 每个生产者都有自己的无锁队列，这样 push 的时候就不用考虑竞争问题了
- 消费者轮流从多个生产者的队列中读取数据

问题的关键是如何轮流从多个消息队列中取消息更好？

ops 是千万级的
```bash
time ./msqueue                                                           
N: 67108864, 67108864
Round: 322952487
error number: 
over
./msqueue  20.55s user 0.05s system 490% cpu 4.195 total
```

mutex 多生产者多消费值，限制消费者为 1 个
ops 是百万级的
```bash
 time ./mutex_mmqueue 
N: 67108864, 67108864
Round: 67363810
./mutex_mmqueue  10.24s user 36.32s system 324% cpu 14.348 total
```

# smqueue

无锁单生产者，多消费者，瓶颈在生产者

同样可以将其改造为多个单生产者 + 单消费者模型
- 多消费者每人维护一个消息队列，这样取的时候就不会存在竞争
- 单个生产者将消息轮流放入多个队列中

还是需要考虑如何轮流放入最佳

```bash
time ./smqueue
N: 16777216, 16777216
Round: 2354919490
N: 16777216N: N: 16777216, 16777216
Round: 2256104938
16777216, 16777216
Round: 2304445098
, 16777216
Round: 2273391085
./smqueue  36.16s user 0.08s system 484% cpu 7.478 total
```

mutex 版 单生产者 + 多消费者
```bash
./mutex_mmqueue
N: 16777216, 16777216
Round: 75099475
N: 16777216, 16777216
Round: 74940848
N: 16777216, 16777216
Round: 75002889
N: 16777216, 16777216
Round: 73791344
./mutex_mmqueue  11.09s user 36.88s system 324% cpu 14.782 total
```

# 再看多生产者 + 多消费者
m 个生产者、n 个消费者，建立 m + n 个队列（单生产者 + 单消费者模型）

生产者考虑消息如何分配到多个管道

消费者考虑从哪个管道读取消息