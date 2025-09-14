#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "server/server.h"

class Server;

class IProcessor
{
public:
    virtual ~IProcessor() = default;
    virtual void process(Server& server, int sock) = 0;
    virtual void show() = 0;
};

#endif
