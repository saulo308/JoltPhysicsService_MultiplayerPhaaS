#ifndef MESSAGEHANDLER_REMOVEBODY_H
#define MESSAGEHANDLER_REMOVEBODY_H

#include "MessageHandlerBase.h"

class MessageHandler_RemoveBody : public MessageHandlerBase
{
public:
    /** 
    * Removes a body from the physics system.
    * The message template should be:
    * 
    * "RemoveBody\n
    * id\n
    * MessageEnd\n"
    * 
    * @param message The received message from the client with the info to 
    * remove the body
    * 
    * @return The result of removing a body. May return a failure message if 
    * could not successfuly remove the body from the physics system
    */
    std::string handleMessage(std::string& message) override;
};

#endif