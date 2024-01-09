#ifndef MESSAGEHANDLER_UPDATEBODYTYPE_H
#define MESSAGEHANDLER_UPDATEBODYTYPE_H

#include "MessageHandlerBase.h"

/** 
* The update body type message handler. Will update a body type on the physics
* system according to the given BodyID.
*/
class MessageHandler_UpdateBodyType : public MessageHandlerBase
{
public:
    /** 
    * Updates a body type on the physics system.
    * The message template should be:
    * 
    * "UpdateBodyType\n
    * id;newBodyType\n
    * MessageEnd\n"
    * 
    * @param message The received message from the client with the info to 
    * update the body type
    * 
    * @return The result of updating the body type. May return a failure 
    * message if could not successfully update the body type on the physics 
    * system
    */
    std::string handleMessage(std::string& message) override;
};

#endif