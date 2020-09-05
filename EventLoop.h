#pragma once

#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>

#include "noncopyable.h"
#include "Timestamp.h"

class Channel;
class Poller;

//事件循环类，其中包括两个主要大模块 Channel Poller(epoll的抽象)
class EventLoop:noncopyable
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    Timestamp pollReturnTime() const { return pollReturnTime_; }

    void runInLoop(Functor cb);
    void queueInLoop(Functor db);
private:
    using ChannelList = std::vector<Channel*>;

    std::atomic_bool looping_;//原子操作，通过CAS实现
    std::atomic_bool quit_;//退出loop循环
    const pid_t threadId_;//当前loop所在线程的id
    Timestamp pollReturnTime_;//记录poll返回发生事件的channels的时间点
    std::unique_ptr<Poller> poller_;

    int wakeupFd_;//当mainLoop获取一个新用户的channel，通过轮训算法选择一个subloop，通过该成员唤醒subloop处理channel
    std::unique_ptr<Channel> wakeupChannel_;//封装wakeupfd_的channel

    ChannelList activeChannels_;
    Channel* currentActiveChannel_;

    std::atomic_bool callPendingFunctor_;//当前loop是否需要执行的回调操作
    std::vector<Functor> pengdingFunctors_;//存储loop需要执行的所有的回调操作
    std::mutex mutex_;//互斥锁用来保护上面vector容器的线程安全操作
};