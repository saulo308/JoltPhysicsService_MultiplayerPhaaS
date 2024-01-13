#include "PhysicsServiceSocketServer.h"
#include "../Communication/MessageHandling/MessageHandlerParser.h"
#include "../Communication/MessageHandling/MessageHandlers/MessageHandler_InitPhysicsSystem.h"
#include "../Communication/MessageHandling/MessageHandlers/MessageHandler_StepPhysicsSystem.h"
#include "../Communication/MessageHandling/MessageHandlers/MessageHandler_RemoveBody.h"
#include "../Communication/MessageHandling/MessageHandlers/MessageHandler_AddBody.h"
#include "../Communication/MessageHandling/MessageHandlers/MessageHandler_UpdateBodyType.h"
#include <sstream>
#include <chrono>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

void PhysicsServiceSocketServer::RunDebugSimulation()
{
    // Creating a new physics service implementation
    physicsServiceImplementation = new PhysicsServiceImpl();

    // Create the physics service message handler parser to parse and
    // delegate incoming messages
    physicsServiceMessageHandlerParser = new MessageHandlerParser();

    // Register all handlers:

    // Register InitPhysicsSystem handler (message type: "Init")
    physicsServiceMessageHandlerParser->registerHandler
        <MessageHandler_InitPhysicsSystem>("Init", 
        physicsServiceImplementation);

    // Register InitPhysicsSystem handler (message type: "Step")
    physicsServiceMessageHandlerParser->registerHandler
        <MessageHandler_StepPhysicsSystem>("Step", 
        physicsServiceImplementation);

    // Register RemoveBody handler (message type: "RemoveBody")
    physicsServiceMessageHandlerParser->registerHandler
        <MessageHandler_RemoveBody>("RemoveBody", 
        physicsServiceImplementation);
    
    // Register AddBody handler (message type: "AddBody")
    physicsServiceMessageHandlerParser->registerHandler
        <MessageHandler_AddBody>("AddBody", 
        physicsServiceImplementation);

    // Register UpdateBodyType handler (message type: "UpdateBodyType")
    physicsServiceMessageHandlerParser->registerHandler
        <MessageHandler_UpdateBodyType>("UpdateBodyType", 
        physicsServiceImplementation);
        
    // Initializing physics system with two spheres and a floor 
    std::string initPhysicsSystemMessage = 
        "Init\n"
        "floor;0;primary;0;0;0\n"
        "sphere;1;primary;0;0;250\n"
        "sphere;2;primary;250;0;250\n"
        "MessageEnd\n";

    // Handle the init message
    physicsServiceMessageHandlerParser->handleMessage(initPhysicsSystemMessage);

    // Testing body removal handler
    std::string removeBodyMessage = 
        "RemoveBody\n"
        "1\n"
        "MessageEnd\n";
    physicsServiceMessageHandlerParser->handleMessage(removeBodyMessage);

    // Testing add body handler (note that the bodyID is not 1, so the body ID
    // 1 should not exist and the bodyID 4 should)
    std::string addBodyMessage = 
        "AddBody\n"
        "sphere;4;primary;0;0;250\n"
        "MessageEnd\n";
    physicsServiceMessageHandlerParser->handleMessage(addBodyMessage);

    // Testing update body type handler
    std::string updateBodyTypeMessage = 
        "UpdateBodyType\n"
        "4;clone\n"
        "MessageEnd\n";
    physicsServiceMessageHandlerParser->handleMessage(updateBodyTypeMessage);

    std::cout << "Steping physics...\n";

    // Execute 30 physics steps
    for(int i = 0; i < 5; i++)
    { 
        std::string stepPhysicsSystemMessage = "Step\nMessageEnd\n";

        // Step physics simulation and get result
        std::string stepSimulationResult = 
            physicsServiceMessageHandlerParser->handleMessage
            (stepPhysicsSystemMessage);
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

    physicsServiceImplementation = new PhysicsServiceImpl();
    currentPhysicsStepSimulationWithoutCommsTimeMeasure = "";

    // Create the physics service message handler parser to parse and
    // delegate incoming messages
    physicsServiceMessageHandlerParser = new MessageHandlerParser();

    // Register all handlers:

    // Register InitPhysicsSystem handler (message type: "Init")
    physicsServiceMessageHandlerParser->registerHandler
        <MessageHandler_InitPhysicsSystem>("Init", 
        physicsServiceImplementation);

    // Register InitPhysicsSystem handler (message type: "Step")
    physicsServiceMessageHandlerParser->registerHandler
        <MessageHandler_StepPhysicsSystem>("Step", 
        physicsServiceImplementation);

    // Register REmoveBody handler (message type: "RemoveBody")
    physicsServiceMessageHandlerParser->registerHandler
        <MessageHandler_RemoveBody>("RemoveBody", 
        physicsServiceImplementation);

    // Register AddBody handler (message type: "AddBody")
    physicsServiceMessageHandlerParser->registerHandler
        <MessageHandler_AddBody>("AddBody", 
        physicsServiceImplementation);

    // Register UpdateBodyType handler (message type: "UpdateBodyType")
    physicsServiceMessageHandlerParser->registerHandler
        <MessageHandler_UpdateBodyType>("UpdateBodyType", 
        physicsServiceImplementation);

    // Receive messages until the peer shuts down the connection
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
            // Append the decoded message as string (the 
            // "messageReceivalReturnValue" indicates the message length)
            decodedMessage += std::string(receivingBuffer, receivingBuffer 
                + messageReceivalReturnValue);
            
            //  (DEBUG) Print received message
            std::cout << "Decoded message:" << decodedMessage << "\n=======\n";

            // While the decoded message does not find "MessageEnd" on the 
            // decoded message, keep appending to the string. This is needed
            // as we may be getting chunks of the actual message
            if(decodedMessage.find("MessageEnd") == std::string::npos)
            {
                continue;
            }

            // Handle the decoded message by passing it to the parser. He will
            // call the proper handler or generate an error if could not find 
            // a proper handler
            std::string messageHandlerReturn = 
                physicsServiceMessageHandlerParser->handleMessage
                (decodedMessage);

            // Send the handler return to the client
            SendMessageToClient(clientSocket, messageHandlerReturn);

            // Empty the current decoded message
            decodedMessage = "";
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
    std::string& messageToSend)
{
    // Check if message does not have "MessageEnd" on it
    if(messageToSend.find("MessageEnd") == std::string::npos)
    {
        // If not, append to it
        messageToSend += "\nMessageEnd\n";
    }

    // Convert the message to send to char*
    const char* messageBuffer = messageToSend.c_str();

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
        file << currentPhysicsStepSimulationWithoutCommsTimeMeasure; 
        file.close(); // Close the file
        std::cout << "Data written to file successfully." << std::endl;
    } 
    else 
    {
        std::cout << "Failed to open the file." << std::endl;
    }
}
