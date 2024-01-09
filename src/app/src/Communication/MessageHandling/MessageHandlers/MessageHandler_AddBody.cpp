#include "MessageHandler_AddBody.h"
#include "../../../PhysicsSimulation/PhysicsServiceImpl.h"

/* 
* Message template:
*
* "AddSphereBody\n
* id; bodyType; posX; posY; posZ\n
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

    // Check for errors
    if (newSphereBodyParsedData.size() < 6)
    {
        std::cout << "Error on parsing addBody message info. Line with less "
            "than 6 params: " << message << '\n';
        return "Error on parsing addBody message info. Line with less "
            "than 6 params.";
    }

    // Get the new sphere body's ID
	const int newSphereId { std::stoi(newSphereBodyParsedData[1]) };

    // Get the new body type
	const std::string newSphereBodyTypeAsString
        { newSphereBodyParsedData[2] };
    // Set the new body type
    EBodyType newBodyType{};
    if (newSphereBodyTypeAsString == "primary")
    {
        newBodyType = EBodyType::Primary;
    }
    else if (newSphereBodyTypeAsString == "clone")
    {
        newBodyType = EBodyType::Clone;
    }
    else
    {
        std::cout << "Unknown body type: " << newSphereBodyTypeAsString << '\n';
    }

    // Get the new sphere body's position
	const double initialPosX { std::stod(newSphereBodyParsedData[3]) };
	const double initialPosY { std::stod(newSphereBodyParsedData[4]) };
	const double initialPosZ { std::stod(newSphereBodyParsedData[5]) };

    // Create data for sphere creation
    const BodyID newSphereBodyID(newSphereId);
    const RVec3 newSphereInitialPos(initialPosX, initialPosY, initialPosZ);

    // Request the creation of sphere
    std::string additionReturn = 
        physicsServiceImplementation->AddNewSphereToPhysicsWorld
        (newSphereBodyID, newBodyType, newSphereInitialPos);

    std::cout << additionReturn << "\n\n";
    return additionReturn;
}
