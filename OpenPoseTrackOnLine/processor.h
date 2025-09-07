#ifndef PROCESSOR_H
#define PROCESSOR_H


class Server;

class IProcessor
{
public:
    virtual ~IProcessor() = default;
    virtual void process(Server& server, int sock) = 0;
};

#endif
