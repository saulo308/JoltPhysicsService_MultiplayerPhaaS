#ifndef MESSAGEHANDLERBASE_H
#define MESSAGEHANDLERBASE_H

#include <iostream>

/** 
* The messge handler base. This is the base for every message handler 
* implementation. This implements base functionality such as setting the
* physics service implementation and processing the incoming message.
*/
class MessageHandlerBase
{
public:
    /** Virtual destructor for non final class */
    virtual ~MessageHandlerBase() = default;

public:
    /** 
    * Initializes the message handler with the physics service implementation.
    * 
    * @param inPhysicsServiceImplementation The physics service implementation
    * reference
    */
    void initializeMessageHandler
        (class PhysicsServiceImpl* inPhysicsServiceImplementation)
    {
        physicsServiceImplementation = inPhysicsServiceImplementation;
    }

    /** 
    * Handles the incoming message. This should be overwritten for each message
    * handler with the proper functionality. However, this base class processes
    * the incoming message to remove the message's first and last line. This is
    * needed as the first line will be the handler type and the last the flag
    * "MessageEnd". Thus, they are unecessary to the message handler processing.
    * 
    * @param message The incoming message to handle
    * 
    * @return The handler response to the message processing. This most likely
    * will be a response from the physics service implementation
    */
    virtual std::string handleMessage(std::string& message);

protected:
    /** 
    * The physics service implementation reference. Used to call the proper
    * methods on the physics service according to the handler.
    */
    class PhysicsServiceImpl* physicsServiceImplementation = nullptr;
};

#endif
