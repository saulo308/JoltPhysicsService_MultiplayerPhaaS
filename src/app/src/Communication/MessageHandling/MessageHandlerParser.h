#ifndef MESSAGEHANDLERPARSER_H
#define MESSAGEHANDLERPARSER_H

#include <iostream>
#include <unordered_map>
#include <memory>
#include "MessageHandlers/MessageHandlerBase.h"

/**
* The message handler parser is responsible for handling the service's incoming
* messages. This will store a set of message handlers that implement the
* proper functionality according to the given message.
*
* Every message must have the handler type on the first line. This type will
* be used here to find the proper handler to the message. This class has a
* map with the registered handlers. The proper handler should exist on the map
* before handling the message.
* 
* @see MessageHandlerBase
*/
class MessageHandlerParser final
{
    /** The handler ptr, stores a ptr to a MessageHandler */
    using HandlerPtr = std::unique_ptr<MessageHandlerBase>;

public:
    /**
    * Handles a incoming message. This will extract the handler type on the
    * message's first line and fint the proper handler on the registered
    * handlers. Then, will pass the message to the proper handler.
    * 
    * @param message The incoming message to be handled
    * 
    * @return The message handler response to the message. This should be used
    * as the response to send the client once the given message has been 
    * processed (e.g. while steping physics will return the step physics
    * response).
    */
    std::string handleMessage(std::string& message);

    /** 
    * Registers handlers on this parser. The handler will be store by the
    * given handler type and a pointer to the given handler object. Upon
    * receiveing a message, the parser will find the proper registered handler
    * on by its type.
    * 
    * @param handlerTypeStr The handler type to register. This should be the
    * string on the first line on each message. This is the str we compare to
    * find the proper handler to the message
    * @param physicsServiceImplementation The physics service implementation
    * ptr. This will be used by the handlers to process the given message
    */
    template <typename T>
    void registerHandler(const std::string& handlerTypeStr, 
        class PhysicsServiceImpl* physicsServiceImplementation) 
    {
        // Create a ptr to the message handler
        messageHandlersMap[handlerTypeStr] = std::make_unique<T>();

        // Initialize the message handler with the physics service 
        // implementation
        messageHandlersMap[handlerTypeStr]->initializeMessageHandler
            (physicsServiceImplementation);
    }

private:
    /**
    * Extracts the handler type from the message. This will get the handler
    * type from the message's first line and return it.
    * 
    * @param message The message to extract the handler type
    * 
    * @return The message's handler type
    */
    std::string extractHandlerTypeFromMessage(const std::string& message);

public:
    /** 
    * The message handlers map. This store as key the handler type and as key
    * a ptr to the handler. We add to this map by registering handlers with the
    * "registerHandler()" method.
    */
    std::unordered_map<std::string, HandlerPtr> messageHandlersMap;
};

#endif
