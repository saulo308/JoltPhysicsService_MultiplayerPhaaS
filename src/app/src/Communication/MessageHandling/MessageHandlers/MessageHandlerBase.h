#ifndef MESSAGEHANDLERBASE_H
#define MESSAGEHANDLERBASE_H

#include <iostream>

class MessageHandlerBase
{
public:
    virtual ~MessageHandlerBase() = default;

public:
    /** */
    virtual void handleMessage(const std::string& message) = 0;

};

#endif
