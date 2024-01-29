#include "MessageHandler_GetSimulationMeasures.h"
#include "../../../PhysicsSimulation/PhysicsServiceImpl.h"

/* 
* Message template:
*
* "GetSimulationMeasures\n
* MessageEnd\n"
*
*/
std::string MessageHandler_GetSimulationMeasures::handleMessage
    (std::string& message)
{
    std::cout << "Get simulation measures requested.\n";

    // Call the base to remove the first and last line
    MessageHandlerBase::handleMessage(message);

    if(!physicsServiceImplementation)
    {
        std::cout << "No physics service implementation valid to get "
            "simulation measures.\n";

        return "No physics service implementation valid to get "
            "simulation measures.";
    }

    // Initialize the physics system with the given info
    std::string simulationMeasures = 
        physicsServiceImplementation->GetSimulationMeasures(); 

    std::cout << "Gotten simulation measures.\n\n";
    std::cout << simulationMeasures << "\n\n";
    return simulationMeasures;
}
