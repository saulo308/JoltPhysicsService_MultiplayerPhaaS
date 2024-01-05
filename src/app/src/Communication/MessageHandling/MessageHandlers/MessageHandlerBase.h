#ifndef MESSAGEHANDLERBASE_H
#define MESSAGEHANDLERBASE_H

#include <iostream>

class MessageHandlerBase
{
public:
    virtual ~MessageHandlerBase() = default;

public:
    /** */
    void initializeMessageHandler
        (class PhysicsServiceImpl* inPhysicsServiceImplementation)
    {
        physicsServiceImplementation = inPhysicsServiceImplementation;
    }

    /** */
    virtual std::string handleMessage(const std::string& message) = 0;

protected:
    /** */
    class PhysicsServiceImpl* physicsServiceImplementation = nullptr;
};

#endif
