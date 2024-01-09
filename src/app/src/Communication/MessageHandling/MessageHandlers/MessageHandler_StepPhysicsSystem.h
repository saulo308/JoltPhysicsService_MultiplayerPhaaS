#ifndef MESSAGEHANDLER_STEPPHYSICSYSTEM_H
#define MESSAGEHANDLER_STEPPHYSICSYSTEM_H

#include "MessageHandlerBase.h"

/** 
* The step physics system message handler. Will step the physics system and 
* return the physics system new state after the step. This will send each 
* actor's Id, position and rotation of the current physics system state back to
* the client.
*/
class MessageHandler_StepPhysicsSystem : public MessageHandlerBase
{
public:
    /** 
    * Steps the current physics system simulation.
    * 
    * The message's template should be:
    * "Step\n
    * MessageEnd\n"
    * 
    * @param message 
    * 
    * @return The step physics simulation result. This will send each actor's
    * Id, position and rotation of the current physics system state back to
    * the client
    */
    std::string handleMessage(std::string& message) override;
};

#endif