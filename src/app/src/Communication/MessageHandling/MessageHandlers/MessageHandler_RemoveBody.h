#ifndef MESSAGEHANDLER_REMOVEBODY_H
#define MESSAGEHANDLER_REMOVEBODY_H

#include "MessageHandlerBase.h"

class MessageHandler_RemoveBody : public MessageHandlerBase
{
public:
    std::string handleMessage(std::string& message) override;
};

#endif