#ifndef MESSAGEHANDLER_ADDBODY_H
#define MESSAGEHANDLER_ADDBODY_H

#include "MessageHandlerBase.h"

class MessageHandler_AddBody : public MessageHandlerBase
{
public:
    /** 
    * Adds a new sphere body to the physics system.
    * The message template should be:
    * 
    * "AddSphereBody\n
    * id; posX; posY; posZ\n
    * MessageEnd\n"
    * 
    * @param message The received message from the client with the info to 
    * create a new sphere body
    * 
    * @return The result of adding a new sphere body. May return a failure 
    * message if could not succesfully add the new sphere body on the physics 
    * system
    */
    std::string handleMessage(std::string& message) override;
};

#endif