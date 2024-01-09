#include "MessageHandler_UpdateBodyType.h"
#include "../../../PhysicsSimulation/PhysicsServiceImpl.h"

/* 
* Message template:
*
"UpdateBodyType\n
*id;newBodyType\n
*MessageEnd\n"
*
*/
std::string MessageHandler_UpdateBodyType::handleMessage
    (std::string& message)
{
    std::cout << "Update body type requested.\n";

    // Call the base to remove the first and last line
    MessageHandlerBase::handleMessage(message);

    if(!physicsServiceImplementation)
    {
        std::cout << "No physics service implementation valid to update body "
            "type.\n";

        return "No physics service implementation valid to update body type.\n";
    }

    // Split info with ";" delimiter
	std::stringstream updateBodyTypeDataStream(message);
	std::vector<std::string> updateBodyTypeParsedData;

	std::string line {};
	while (std::getline(updateBodyTypeDataStream, line, ';')) 
	{
		updateBodyTypeParsedData.push_back(line);
	}

    // Check for errors
    if (updateBodyTypeParsedData.size() < 2)
    {
        std::cout << "Error on parsing update body type message info. Line "
            "with less than 2 params: " << message << '\n';
        return "Error on parsing update body type message info. Line with less "
            "than 2 params.";
    }

    // Get the body id to update from the message
	const int bodyIdToUpdateAsInt { std::stoi(updateBodyTypeParsedData[0]) };

    // Create the BodyID
    const BodyID bodyIdToUpdate(bodyIdToUpdateAsInt);

    // Get the new body type
	const std::string newBodyTypeAsString
        { updateBodyTypeParsedData[1] };
    // Set the new body type
    EBodyType newBodyType{};
    if (newBodyTypeAsString == "primary")
    {
        newBodyType = EBodyType::Primary;
    }
    else if (newBodyTypeAsString == "clone")
    {
        newBodyType = EBodyType::Clone;
    }
    else
    {
        std::cout << "Unknown body type: " << newBodyTypeAsString << '\n';
    }

    // Request the body type update
    std::string updateBodyReturn = 
        physicsServiceImplementation->UpdateBodyType(bodyIdToUpdate, 
        newBodyType);

    std::cout << updateBodyReturn << "\n\n";
    return updateBodyReturn;
}
