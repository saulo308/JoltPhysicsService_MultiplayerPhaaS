#include "MessageHandler_InitPhysicsSystem.h"
#include "../../../PhysicsSimulation/PhysicsServiceImpl.h"

std::string MessageHandler_InitPhysicsSystem::handleMessage(const std::string &message)
{
    std::cout << "Initialize physics system requested.\n";

    if(!physicsServiceImplementation)
    {
        std::cout << "No physics service implementation valid to init physics system.\n";
        return "No physics service implementation valid to init physics system.\n";
    }

    // Initialize the physics system with the given info
    physicsServiceImplementation->InitPhysicsSystem(message);

    std::cout << "Physics system initialized.\n";
    return "Physics system initialized.\n";
}
