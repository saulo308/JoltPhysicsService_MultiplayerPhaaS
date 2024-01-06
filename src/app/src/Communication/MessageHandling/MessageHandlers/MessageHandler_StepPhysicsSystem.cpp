#include "MessageHandler_StepPhysicsSystem.h"
#include "../../../PhysicsSimulation/PhysicsServiceImpl.h"

std::string MessageHandler_StepPhysicsSystem::handleMessage
    (std::string& message)
{
    std::cout << "Step physics system requested.\n";

    // Call the base to remove the first and last line
    MessageHandlerBase::handleMessage(message);

    if(!physicsServiceImplementation)
    {
        std::cout << "No physics service implementation valid to step physics system.\n";
        return "No physics service implementation valid to step physics system.";
    }

    // Step the physics system 
    std::string stepPhysicsResult = 
        physicsServiceImplementation->StepPhysicsSimulation(); 

    std::cout << "Physics system step finished.\n\n";
    return stepPhysicsResult;
}
