#include "MessageHandler_AddBody.h"
#include "../../../PhysicsSimulation/PhysicsServiceImpl.h"

/* 
* Message template:
*
* "AddSphereBody\n
* id; posX; posY; posZ\n
* MessageEnd\n"
*
*/
std::string MessageHandler_AddBody::handleMessage
    (std::string& message)
{
    std::cout << "New sphere body addition requested. Processing...\n";

    // Call the base to remove the first and last line
    MessageHandlerBase::handleMessage(message);

    if(!physicsServiceImplementation)
    {
        std::cout << "No physics service implementation valid to add new sphere"
            "body.\n";

        return "Error: Could not create sphere as physics service "
            "implementation is null.";
    }

	// Split info with ";" delimiter
	std::stringstream newSphereBodyDataStream(message);
	std::vector<std::string> newSphereBodyParsedData;

	std::string line {};
	while (std::getline(newSphereBodyDataStream, line, ';')) 
	{
		newSphereBodyParsedData.push_back(line);
	}

    // Get the new sphere body's ID
	const int newSphereId = std::stoi(newSphereBodyParsedData[0]);

    // Get the new sphere body's position
	const double initialPosX = std::stod(newSphereBodyParsedData[1]);
	const double initialPosY = std::stod(newSphereBodyParsedData[2]);
	const double initialPosZ = std::stod(newSphereBodyParsedData[3]);

    // Create data for sphere creation
    const BodyID newSphereBodyID(newSphereId);
    const RVec3 newSphereInitialPos(initialPosX, initialPosY, initialPosZ);

    // Request the creation of sphere
    std::string additionReturn = 
        physicsServiceImplementation->AddNewSphereToPhysicsWorld
        (newSphereBodyID, newSphereInitialPos);

    std::cout << additionReturn << "\n\n";
    return additionReturn;
}
