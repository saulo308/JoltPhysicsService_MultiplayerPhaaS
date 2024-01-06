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
    std::string handleMessage(std::string& message);

    template <typename T>
    void register_handler(const std::string& messageType, 
        class PhysicsServiceImpl* physicsServiceImplementation) 
    {
        messageHandlersMap[messageType] = std::make_unique<T>();
        messageHandlersMap[messageType]->initializeMessageHandler
            (physicsServiceImplementation);
    }

private:
    std::string extractHandlerTypeFromMessage(const std::string& message);

public:
    std::unordered_map<std::string, HandlerPtr> messageHandlersMap;

};

#endif
