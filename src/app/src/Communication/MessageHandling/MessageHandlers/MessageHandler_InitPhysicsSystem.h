#ifndef MESSAGEHANDLER_INITPHYSICSYSTEM_H
#define MESSAGEHANDLER_INITPHYSICSYSTEM_H

#include "MessageHandlerBase.h"

class MessageHandler_InitPhysicsSystem : MessageHandlerBase
{
public:
    void handleMessage(const std::string& message) override;
};

#endif