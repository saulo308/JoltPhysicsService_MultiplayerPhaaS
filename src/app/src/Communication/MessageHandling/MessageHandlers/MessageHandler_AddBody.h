#ifndef MESSAGEHANDLER_ADDBODY_H
#define MESSAGEHANDLER_ADDBODY_H

#include "MessageHandlerBase.h"

/** 
* The add body message handler. Will add a body according to the given data on
* the physics system.
*/
class MessageHandler_AddBody : public MessageHandlerBase
{
public:
    /** 
    * Adds a new sphere body to the physics system.
    * The message template should be:
    * 
    * "AddBody\n
    * actorType; id_0; bodyType; posX_0; posY_0; posZ_0\n
    * MessageEnd\n"
    * 
    * @param message The received message from the client with the info to 
    * create a new sphere body
    * 
    * @return The result of adding a new sphere body. May return a failure 
    * message if could not successfully add the new sphere body on the physics 
    * system
    */
    std::string handleMessage(std::string& message) override;
};

#endif