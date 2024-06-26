#include "MessageHandlerParser.h"

std::string MessageHandlerParser::handleMessage(std::string& message)
{
    // Extracting the handler type from the message
    const std::string handlerType = extractHandlerTypeFromMessage(message);

    // Find the handler on the map
    auto handlerPtr = messageHandlersMap.find(handlerType);

    // If could find the handler, handle the message
    if(handlerPtr != messageHandlersMap.end())
    {
        return handlerPtr->second->handleMessage(message);
    }

    // If not, call the unknown message method
    printf("Error: Message type could not be handled. Message: (%s).\n"
        "Every receving message should have the message type on the first"
        "line. The given handler type is unknown.\n\n", message.c_str());

    return "Error: Message type could not be handled.";
}

std::string MessageHandlerParser::extractHandlerTypeFromMessage
    (const std::string& message)
{
    // Get the message type delimiter ('\n') every message should have the
    // handler type on the first line
    const size_t messageTypeDelimiterPos = message.find('\n');

    // If found the delimiter, return the substring from the message init into 
    // it
    if(messageTypeDelimiterPos != std::string::npos)
    {
        return message.substr(0, messageTypeDelimiterPos);
    }

    // If not, just return the message itself
    return message;
}
