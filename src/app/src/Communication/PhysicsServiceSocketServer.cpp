#include "PhysicsServiceSocketServer.h"
#include <sstream>
#include <chrono>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

void PhysicsServiceSocketServer::RunDebugSimulation()
{
    // Creating a new physics service implementation
    PhysicsServiceImplementation = new PhysicsServiceImpl();

    // Initializing physics system with two spheres and a floor 
    const std::string test = 
        "Init\n"
        "floor;0;0;0;0\n"
        "sphere;1;0;0;250\n"
        "sphere;2;250;0;250\n"
        "EndMessage\n";
    InitializePhysicsSystem(test);

    std::cout << "Steping physics...\n";

    // Execute 30 physics steps
    int step = 0;
    for(int i = 0; i < 30; i++)
    {
        // Step physics simulation and get result
        std::string stepSimulationResult = StepPhysicsSimulation();

        // Print result
        std::cout << "Step(" << step++ << "): \n" 
            << stepSimulationResult << '\n';
    }
}

bool PhysicsServiceSocketServer::OpenServerSocket(const char* serverPort)
{
    // Get this server (local) addrinfo
    // This will get the server addr as localhost
    addrinfo hints, *addrInfoResult;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    int getAddrInfoReturnValue = getaddrinfo(NULL, serverPort, &hints,
        &addrInfoResult);
    if (getAddrInfoReturnValue != 0)
    {
        printf("getaddrinfo failed with error: %s\n", 
            gai_strerror(getAddrInfoReturnValue));
        return false;
    }
    
    // Create a socket for the server to listen for client connections.
    int serverListenSocket = CreateListenSocket(addrInfoResult);
    if (serverListenSocket == -1) 
    {
        freeaddrinfo(addrInfoResult);
        return false;
    }

    // Setup the TCP listening socket
    if(!BindListenSocket(serverListenSocket, addrInfoResult))
    {
        freeaddrinfo(addrInfoResult);
        return false;
    }

    // Free addrinfo as we don't need it anymore
    freeaddrinfo(addrInfoResult);

    // Await for the client connection on the listening socket
    int clientSocket = AwaitClientConnection(serverListenSocket);
    if (clientSocket == -1) 
    {
        return false;
    }

    // Once the connection has been established, close the listening socket 
    // as we won't need it anymore (the communication is done via the new 
    // client socket)
    close(serverListenSocket);

    PhysicsServiceImplementation = new PhysicsServiceImpl();
    CurrentPhysicsStepSimulationWithoutCommsTimeMeasure = "";

    // Receive until the peer shuts down the connection
    ssize_t messageReceivalReturnValue = 0;
    do 
    {
        std::cout << "Awaiting client message...\n";

        // Will stall this process thread until receives a new message from 
        // client.
        // The message will be passed on the buffer and the amount of received 
        // bytes is the return value
        // @note Passing a default buffer len. For bigger messages, this should 
        // be increased
        char receivingBuffer[DEFAULT_BUFLEN];
        messageReceivalReturnValue = ReceiveMessageFromClient(clientSocket, 
            receivingBuffer, DEFAULT_BUFLEN);
        if(messageReceivalReturnValue <= 0)
        {
            break;
        }

        // Debug: Print received message
        if(messageReceivalReturnValue > 0)
        {
            //  (DEBUG) Print received message
            decodedMessage += std::string(receivingBuffer, receivingBuffer 
                + messageReceivalReturnValue);
            std::cout << "Decoded message:" << decodedMessage << "\n=======\n";

            if((decodedMessage.find("Init") != std::string::npos) 
                && (decodedMessage.find("EndMessage") != std::string::npos))
            {
                // Initialize the physics system
                InitializePhysicsSystem(decodedMessage);

                // Send client the confirmation
                SendMessageToClient(clientSocket, "MessageEnd");
                decodedMessage = "";
                continue;
            }

            if(decodedMessage.find("Step") != std::string::npos)
            {
                // Get pre step physics time
                std::chrono::steady_clock::time_point preStepPhysicsTime = 
                    std::chrono::steady_clock::now();

                // Step the physics world and get result
                std::string stepSimulationResult = StepPhysicsSimulation();
                stepSimulationResult += "MessageEnd\n";

                // Get post physics communication time
                std::chrono::steady_clock::time_point postStepPhysicsTime = 
                    std::chrono::steady_clock::now();

                // Calculate the microsseconds all step physics simulation
                // (considering communication )took
                std::stringstream ss;
                ss << std::chrono::duration_cast<std::chrono::microseconds>
                    (postStepPhysicsTime - preStepPhysicsTime).count();
                const std::string elapsedTime = ss.str();

                // Append the delta time to the current step measurement
                CurrentPhysicsStepSimulationWithoutCommsTimeMeasure 
                    += elapsedTime + "\n";

                // Send step physics result to client
                SendMessageToClient(clientSocket, 
                    stepSimulationResult.c_str());
                decodedMessage = "";

                continue;
            }

            // Check if we should add a new sphere body
            if(decodedMessage.find("AddSphereBody") != std::string::npos)
            {
                // Add new sphere body (the method will parse the string)
                auto SphereBodyAddOperationResult = 
                    AddNewSphereBody(decodedMessage);
                SphereBodyAddOperationResult += "MessageEnd\n";

                // Send operation result to client
                SendMessageToClient(clientSocket, 
                    SphereBodyAddOperationResult.c_str());

                // Clear received message
                decodedMessage = "";
                continue;
            }

            // Check if we should remove a body
            if(decodedMessage.find("RemoveBody") != std::string::npos)
            {
                // Remove the body (the method will parse the string)
                auto BodyRemovalOperationResult = 
                    RemoveBody(decodedMessage);
                BodyRemovalOperationResult += "MessageEnd\n";

                // Send operation result to client
                SendMessageToClient(clientSocket, 
                    BodyRemovalOperationResult.c_str());

                // Clear received message
                decodedMessage = "";
                continue;
            }

            std::cout << "Unknown message: " << decodedMessage << std::endl;
            SendMessageToClient(clientSocket, "Unkown message error");
        }
    } while (messageReceivalReturnValue > 0);

    // Save step physics measurement to file
    SaveStepPhysicsMeasureToFile();

    // shutdown the connection since we're done
    const int shutdownResult = shutdown(clientSocket, SHUT_RDWR);
    if (shutdownResult == -1) 
    {
        printf("Shutdown failed with error: %s\n", strerror(errno));
        close(clientSocket);
        return false;
    }

    // Finished work, clean up
    close(clientSocket);

    return true;
}

int PhysicsServiceSocketServer::CreateListenSocket
    (addrinfo* listenSocketAddrInfo)
{
    // Create a new socket using the addr info 
    int newListenSocket = socket(listenSocketAddrInfo->ai_family, 
        listenSocketAddrInfo->ai_socktype, listenSocketAddrInfo->ai_protocol);

    // Check if creation was successful
    if (newListenSocket == -1) 
    {
        printf("Socket failed with error: %s\n", strerror(errno));
        return -1;
    }

    return newListenSocket;
}

bool PhysicsServiceSocketServer::BindListenSocket
    (int listenSocketToSetup, addrinfo* listenSocketAddrInfo)
{
    // Bind the listen socket to the addrinfo
    const int bindReturnValue = 
        bind(listenSocketToSetup, listenSocketAddrInfo->ai_addr, 
        listenSocketAddrInfo->ai_addrlen);

    // Check for errors
    if (bindReturnValue == -1) 
    {
        printf("Bind failed with error: %s\n", strerror(errno));
        close(listenSocketToSetup);
        return false;
    }

    return true;
}

int PhysicsServiceSocketServer::AwaitClientConnection(int listenSocket)
{
    const int listenReturnValue = 
        listen(listenSocket, SOMAXCONN);

    if (listenReturnValue == -1) 
    {
        printf("Listen failed with error: %s\n", strerror(errno));
        close(listenSocket);
        return -1;
    }

    // Await a client connection to the listening socket
    printf("Awaiting client connection...\n");

    // Once connection is done, the library will create a new socket for it
    int connectedClientSocket = accept(listenSocket, NULL, NULL);

    // Check for errors on the client socket creation
    if (connectedClientSocket == -1) 
    {
        printf("Socket accept failed with error: %s\n", strerror(errno));
        close(listenSocket);
        return -1;
    }

    return connectedClientSocket;
}

ssize_t PhysicsServiceSocketServer::ReceiveMessageFromClient(int clientSocket,
    char* receivingBuffer, int receivingBufferLength)
{
    // This call will stall this process thread until we receive a message 
    // from the client (game)
    // The message received will be on "receivingBuffer", given the buffer 
    // length
    // The returning value will be the amount of bytes on the received message
    const ssize_t bytesReceivedAmount =
        recv(clientSocket, receivingBuffer, receivingBufferLength, 0);

    // If received 0, that means the client is requesting to close the 
    // connection
    if(bytesReceivedAmount == 0)
    {
        printf("Received a close connection message (0 bytes)\n");
        printf("Closing connection...\n");
        return 0;
    }

    // If received a value > 0, we have a valid message from the client
    if(bytesReceivedAmount > 0)
    {
        //printf("Received bytes amount: %ld\n", bytesReceivedAmount);

        // return the amount of received bytes
        return bytesReceivedAmount;
    }

    // If received value is < 0, we have an error. Let's close the connection
    printf("recv failed with error: %s\n", strerror(errno));
    close(clientSocket);

    return -1;
}

bool PhysicsServiceSocketServer::SendMessageToClient(int clientSocket, 
    const char* messageBuffer)
{
    // Send the given message to the client
    const ssize_t sendReturnValue = send(clientSocket, messageBuffer, 
        strlen(messageBuffer), 0);

    // Check for sending error
    if (sendReturnValue == -1) 
    {
        printf("send failed with error: %s\n", strerror(errno));
        close(clientSocket);
        return false;
    }

    printf("Message sent: %s\n", messageBuffer);
    printf("Bytes sent: %ld\n", sendReturnValue);
    return true;
}

void PhysicsServiceSocketServer::InitializePhysicsSystem
    (const std::string initializationActorsInfo)
{
    if(!PhysicsServiceImplementation)
    {
        std::cout << "No physics service implementation valid to init physics system.\n";
        return;
    }

    PhysicsServiceImplementation->InitPhysicsSystem(initializationActorsInfo);
}

/** 
* Message template:
*
* "Step"
*/
std::string PhysicsServiceSocketServer::StepPhysicsSimulation()
{
    if(!PhysicsServiceImplementation)
    {
        std::cout << "No physics service implementation valid to step physics simulation.\n";
        return "";
    }

    return PhysicsServiceImplementation->StepPhysicsSimulation();
}

/** 
* Message template:
*
* "AddSphereBody\n
* id; posX; posY; posZ; rotX; rotY; rotZ"
*
*/
std::string PhysicsServiceSocketServer::AddNewSphereBody
    (const std::string decodedMessageWithNewBodyInfo)
{
    std::cout << "New sphere body addition requested. Processing...\n";

    if(!PhysicsServiceImplementation)
    {
        std::cout << "No physics service implementation valid to add new sphere body.\n";
        return "Error: Could not create sphere as physics service implementation is null.";
    }

    // Split decoded message into lines
	std::stringstream decodedMessageStringStream
        (decodedMessageWithNewBodyInfo);
    std::vector<std::string> newBodyDataLines;

	std::string line;
    while (std::getline(decodedMessageStringStream, line)) 
	{
        newBodyDataLines.push_back(line);
    }

    // Check if line has the right amount of lines
    if(newBodyDataLines.size() != 2)
    {
        std::cout << "New sphere creation decoded message does not have the right amount of data.\n";
        return "Error: Could not create sphere as decoded message does not contain the right amount of data.";
    }

    // Get the second line as it contain the new sphere's creation data
    // ("id; posX; posY; posZ; rotX; rotY; rotZ")
    const std::string newSphereBodyData = newBodyDataLines[1];

	// Split info with ";" delimiter
	std::stringstream newSphereBodyDataStream(newSphereBodyData);
	std::vector<std::string> newSphereBodyParsedData;

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
        PhysicsServiceImplementation->AddNewSphereToPhysicsWorld
        (newSphereBodyID, newSphereInitialPos);

    std::cout << additionReturn;
    return additionReturn;
}   

/** 
* Message template:
*
* "RemoveBody\n
* id"
*/
std::string PhysicsServiceSocketServer::RemoveBody
    (const std::string decodedMessageWithRemoveBodyInfo)
{
    std::cout << "Remove body requested. Processing...\n";

    if(!PhysicsServiceImplementation)
    {
        std::cout << "No physics service implementation valid to remove body.\n";
        return "Error: Could not remove body as physics service implementation is null.";
    }

    // Split decoded message into lines
	std::stringstream decodedMessageStringStream
        (decodedMessageWithRemoveBodyInfo);
    std::vector<std::string> newBodyDataLines;

	std::string line;
    while (std::getline(decodedMessageStringStream, line)) 
	{
        newBodyDataLines.push_back(line);
    }

    // Check if line has the right amount of lines
    if(newBodyDataLines.size() != 2)
    {
        std::cout << "Remove body decoded message does not have the right amount of data.\n";
        return "Error: Could not remove body as decoded message does not contain the right amount of data.";
    }

    // Get the second line as it contain the new sphere's creation data
    // ("id")
    const std::string bodyToRemoveData = newBodyDataLines[1];

    // Get the requested body's ID for removal
	const int bodyIdToRemoveAsInt = std::stoi(bodyToRemoveData);

    // Convert the id into BodyId
    const BodyID bodyIdToRemove(bodyIdToRemoveAsInt);

    std::cout << "Requesting physics service to remove body...\n";

    // Request the creation of sphere
    std::string removalReturn = 
        PhysicsServiceImplementation->RemoveBodyByID(bodyIdToRemove);

    std::cout << removalReturn;
    return removalReturn;
}

void PhysicsServiceSocketServer::SaveStepPhysicsMeasureToFile()
{
    std::string directoryName = "StepPhysicsMeasure";

    // Create the directory
    fs::create_directory(directoryName);

    std::string fileName = 
        "/StepPhysicsMeasureWithoutCommsOverhead_Remote_Spheres_.txt";
    std::string fullPath = directoryName + "/" + fileName;

    // Open the file in output mode
    std::ofstream file(fullPath);
    
    // Check if the file was opened successfully
    if (file.is_open()) 
    { 
        // Write the string to the file
        file << CurrentPhysicsStepSimulationWithoutCommsTimeMeasure; 
        file.close(); // Close the file
        std::cout << "Data written to file successfully." << std::endl;
    } 
    else 
    {
        std::cout << "Failed to open the file." << std::endl;
    }
}
