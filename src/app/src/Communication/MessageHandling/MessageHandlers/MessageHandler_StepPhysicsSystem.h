#ifndef MESSAGEHANDLER_STEPPHYSICSYSTEM_H
#define MESSAGEHANDLER_STEPPHYSICSYSTEM_H

#include "MessageHandlerBase.h"

class MessageHandler_StepPhysicsSystem : public MessageHandlerBase
{
public:
    std::string handleMessage(std::string& message) override;
};

#endif