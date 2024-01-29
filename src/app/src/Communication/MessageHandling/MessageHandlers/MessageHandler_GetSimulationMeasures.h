#ifndef MESSAGEHANDLER_GETSIMULATIONMEASURES_H
#define MESSAGEHANDLER_GETSIMULATIONMEASURES_H

#include "MessageHandlerBase.h"

/**
 * 
*/
class MessageHandler_GetSimulationMeasures : public MessageHandlerBase
{
public:
    /** 
    *
    */
    std::string handleMessage(std::string& message) override;
};

#endif