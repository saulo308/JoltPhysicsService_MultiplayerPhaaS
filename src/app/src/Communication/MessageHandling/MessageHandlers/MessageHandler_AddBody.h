#ifndef MESSAGEHANDLER_ADDBODY_H
#define MESSAGEHANDLER_ADDBODY_H

#include "MessageHandlerBase.h"

class MessageHandler_AddBody : public MessageHandlerBase
{
public:
    std::string handleMessage(std::string& message) override;
};

#endif