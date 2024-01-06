#ifndef MESSAGEHANDLER_INITPHYSICSYSTEM_H
#define MESSAGEHANDLER_INITPHYSICSYSTEM_H

#include "MessageHandlerBase.h"

class MessageHandler_InitPhysicsSystem : public MessageHandlerBase
{
public:
    /** 
    * Initializes a physics system. On the param, could receive initial bodies
    * to create on the physics system.
    * 
    * The message's template should be:
    * "Init\n
    * Id_0; posX_0; posY_0; posZ_0\n
    * Id_1; posX_1; posY_1; posZ_1\n
    * Id_2; posX_2; posY_2; posZ_2\n
    * ...
    * MessageEnd"
    * 
    * @param message The received message from the client with the physics
    * system initialization system
    * 
    * @return 
    */
    std::string handleMessage(std::string& message) override;
};

#endif