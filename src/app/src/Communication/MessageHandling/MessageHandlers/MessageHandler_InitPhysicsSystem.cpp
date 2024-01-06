#include "MessageHandler_InitPhysicsSystem.h"
#include "../../../PhysicsSimulation/PhysicsServiceImpl.h"

std::string MessageHandler_InitPhysicsSystem::handleMessage
    (std::string& message)
{
    std::cout << "Initialize physics system requested.\n";

    // Call the base to remove the first and last line
    MessageHandlerBase::handleMessage(message);

    if(!physicsServiceImplementation)
    {
        std::cout << "No physics service implementation valid to init physics system.\n";
        return "No physics service implementation valid to init physics system.";
    }

    // Initialize the physics system with the given info
    physicsServiceImplementation->InitPhysicsSystem(message); 

    std::cout << "Physics system initialized.\n";
    return "Physics system initialized.";
}
