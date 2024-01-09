#ifndef MESSAGEHANDLER_REMOVEBODY_H
#define MESSAGEHANDLER_REMOVEBODY_H

#include "MessageHandlerBase.h"

/** 
* The remove body message handler. Will remove a body according to the given
* BodyID from the physics system.
*/
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
    * could not successfully remove the body from the physics system
    */
    std::string handleMessage(std::string& message) override;
};

#endif