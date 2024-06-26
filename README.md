# Concurrency

# ssqueue(single producer single consumer)
无锁版单生产者单消费者，ops 在千万级别(有锁版百万级别)
```bash
time ./ssqueue 
N: 33554432, 33554432
Round: 104362965
./ssqueue  3.98s user 0.00s system 199% cpu 1.996 total
```

实现时注意两点：
- push 操作：数据没有完全加入队列，绝对不能更新 “写下标”
- pop 操作：数据没有完全取出，绝对不能更新 “读下标”


# msqueue(multiple producer single consumer)
多生产者 + 单消费者，瓶颈在单消费者

可以将其拆分为多个 单生产者 + 单消费者模型
- 每个生产者都有自己的无锁队列，这样 push 的时候就不用考虑竞争问题了
- 消费者轮流从多个生产者的队列中读取数据

避免了锁也引入了新的问题：如何轮流从多个消息队列中取消息更好？


# smqueue(single producer multiple consumer)

单生产者 + 多消费者，瓶颈在生产者

同样可以将其改造为多个单生产者 + 单消费者模型
- 多消费者每人维护一个消息队列，这样取的时候就不会存在竞争
- 单个生产者将消息轮流放入多个队列中

避免了锁也引入了新的问题：如何轮流放入最佳

# mmqueue(multiple producer multiple consumer)

## mutex_mmqueue
互斥锁版多生产者多消费者，多线程(4 + 4)协同千万级别
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

## fake_lock_free_queue
无锁版多生产者多消费者。额，这效率...... 主要是由于当前实现有两个致命缺陷（详见 code）

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

## lock_free_queue
“m 个生产者 + n 个消费者” 看成 m * n 个 “单生产者 + 单消费者”, 建立 m * n 个 ssqueue


避免了加锁但也引入了新的问题：
- 生产者考虑消息如何分配到多个队列
- 消费者考虑如何从多个队列取得消息

```bash
time ./lock_free_mmqueue
4 producers will send 67108864 messages totally
produce msgs 16777216 in 17695903 rounds
produce msgs 16777216 in 17630286 rounds
produce msgs 16777216 in 17590456 rounds
produce msgs 16777216 in 17426067 rounds
consume msgs 16714528 in 102152074 rounds
consume msgs 16978476 in 111410669 rounds
consume msgs 16700181 in 104285372 rounds
consume msgs 16715679 in 109422734 rounds
4 consumers received 67108864 messages totally
./lock_free_mmqueue  13.20s user 0.03s system 660% cpu 2.003 total

```
