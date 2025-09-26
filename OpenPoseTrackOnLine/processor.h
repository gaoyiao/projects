#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "server/server.h"

class Server;

/*
    written by gaoyiao 2025/9/26 10:51

    
    考虑高并发，大量客户端情况
    TODO:
    假设server中有多个Nvidia GPU(假设有8块（0 ~ 7））， 当单个GPU负载过重，实现GPU调度
    实现GPU调度：假如有多个用户客户端连接同一个Server，实现GPU调度策略：
    1. 让一个用户独占8块GPU，采用分布式推理（8块并行，N个CUDA核心并行）？ 每个客户端（设置时间片轮转， 优先级队列）尽可能保证每个客户端获得相同时间的GPU资源）
    2. 为更多的客户端提供计算资源，让每个客户端获得更细粒度的资源使用时间，可能会带来CPU调度的开销（频繁切换客户端）
    3. Thinking ... 
    4. Thinking ...
*/

class IProcessor
{
public:
    virtual ~IProcessor() = default;
    virtual void process(Server& server, int sock) = 0;
    virtual void show() = 0;
};

#endif
