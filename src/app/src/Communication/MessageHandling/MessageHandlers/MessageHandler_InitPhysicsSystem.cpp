#include "MessageHandler_InitPhysicsSystem.h"
#include "../../../PhysicsSimulation/PhysicsServiceImpl.h"

/* 
* Message template:
*
* "Init\n
* Id_0; bodyType; posX_0; posY_0; posZ_0\n
* Id_1; bodyType; posX_1; posY_1; posZ_1\n
* Id_2; bodyType; posX_2; posY_2; posZ_2\n
* ...
* MessageEnd\n"
*
*/
std::string MessageHandler_InitPhysicsSystem::handleMessage
    (std::string& message)
{
    std::cout << "Initialize physics system requested.\n";

    // Call the base to remove the first and last line
    MessageHandlerBase::handleMessage(message);

    if(!physicsServiceImplementation)
    {
        std::cout << "No physics service implementation valid to init physics "
            "system.\n";

        return "No physics service implementation valid to init physics "
            "system.";
    }

    // Initialize the physics system with the given info
    physicsServiceImplementation->InitPhysicsSystem(message); 

    std::cout << "Physics system initialized.\n\n";
    return "Physics system initialized.";
}
