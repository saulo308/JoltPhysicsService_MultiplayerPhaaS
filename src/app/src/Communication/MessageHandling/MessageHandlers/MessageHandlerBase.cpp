#include "MessageHandlerBase.h"


std::string MessageHandlerBase::handleMessage(std::string& message)
{
    // Get the first and last line '\n' character position
    const size_t firstLinePos = message.find('\n');
    const size_t lastLinePos = message.rfind('\n');
    const size_t secondLastLinePos = message.rfind('\n', lastLinePos - 1);

    // Remove the first and last line. This is needed as the first line will
    // be the type of the handler and the last will be "MessageEnd"
    if(firstLinePos != std::string::npos && lastLinePos != std::string::npos)
    {
        // Calculate the substring initial pos, starting from the second line
        const size_t substringInitialPos = firstLinePos + 1;

        // Calculate the substring length, which will be the second last
        // end pos minus de first line pos
        const size_t substringLength = secondLastLinePos - firstLinePos - 1;

        // Set the message with the substring without the first and last line
        message = message.substr(substringInitialPos, substringLength);
    }

    return "";
}
