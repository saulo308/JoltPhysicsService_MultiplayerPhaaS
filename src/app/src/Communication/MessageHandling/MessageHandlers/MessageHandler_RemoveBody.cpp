#include "MessageHandler_RemoveBody.h"
#include "../../../PhysicsSimulation/PhysicsServiceImpl.h"

/* 
* Message template:
*
* "RemoveBody\n
* id\n
* MessageEnd\n"
*
*/
std::string MessageHandler_RemoveBody::handleMessage
    (std::string& message)
{
    std::cout << "Remove body requested. Processing...\n";

    // Call the base to remove the first and last line
    MessageHandlerBase::handleMessage(message);

    if(!physicsServiceImplementation)
    {
        std::cout << "No physics service implementation valid to remove "
            "body.\n";

        return "Error: Could not remove body as physics service implementation "
            "is null.";
    }

    // Get the requested body's ID for removal
	const int bodyIdToRemoveAsInt = std::stoi(message);

    // Convert the id into BodyId
    const BodyID bodyIdToRemove(bodyIdToRemoveAsInt);

    std::cout << "Requesting physics service to remove body.\n";

    // Request the creation of sphere
    std::string removalReturn = 
        physicsServiceImplementation->RemoveBodyByID(bodyIdToRemove);

    std::cout << removalReturn << "\n\n";
    return removalReturn;
}
