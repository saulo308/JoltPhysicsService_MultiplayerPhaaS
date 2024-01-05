#ifndef MESSAGEHANDLERPARSER_H
#define MESSAGEHANDLERPARSER_H

#include <iostream>
#include <unordered_map>
#include <memory>
#include "MessageHandlers/MessageHandlerBase.h"

class MessageHandlerParser final
{
    using HandlerPtr = std::unique_ptr<MessageHandlerBase>;

public:
    void handleMessage(const std::string& message);

    template <typename T>
    void register_handler(const std::string& messageType) 
    {
        messageHandlersMap[messageType] = std::make_unique<T>();
    }

private:
    std::string extractHandlerTypeFromMessage(const std::string& message);

public:
    std::unordered_map<std::string, HandlerPtr> messageHandlersMap;

};

#endif
