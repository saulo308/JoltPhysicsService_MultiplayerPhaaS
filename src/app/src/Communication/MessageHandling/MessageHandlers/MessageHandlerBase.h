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
    virtual std::string handleMessage(std::string& message);

protected:
    /** */
    class PhysicsServiceImpl* physicsServiceImplementation = nullptr;
};

#endif
