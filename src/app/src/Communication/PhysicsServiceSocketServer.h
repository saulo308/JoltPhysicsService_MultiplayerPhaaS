#ifndef SOCKERSERVER_H
#define SOCKERSERVER_H

#include <iostream>
#include <cstring>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include "../PhysicsSimulation/PhysicsServiceImpl.h"

#define DEFAULT_BUFLEN 1048576

/** 
* This class is responsible for opening a socket server. Thus, will act as the
* physics service server. This will open the socket connection on the given
* port and await client's messages. The received messages will be processed
* and delegated to the physics system. Once the physics system process the
* requested functionality, will send the client a response.
*/
class PhysicsServiceSocketServer
{
public:
    /** 
    * Runs a debug simulation. This will simulate a init and step physics
    * message so we can use it for debug purposes.
    */
    void RunDebugSimulation();

    /** 
    * Opens the server socket. The server port to open the socket is given by
    * the param. 
    * 
    * Moreover, this method will keep a loop to receive client's messages until
    * he requests to close this server. The received messages will be processed
    * and the requested functionality will be delegated to the physics service
    * implementation.
    * 
    * @param serverPort The port to open the server on
    * 
    * @return True if could succesfully open the socket on the given port and
    * false otherwise.
    */
    bool OpenServerSocket(const char* serverPort);

private:
    /** 
    * Creates a listen socket on the given addrinfo. This socket will await a
    * client connection
    * 
    * @param listenSocketAddrInfo The addrinfo to open the listen socket on
    * 
    * @param File descriptor for opened socket or -1 if any error has occured
    * while creating the socket.
    */
    int CreateListenSocket(addrinfo* listenSocketAddrInfo);
    
    /** 
    * Binds a given listen socket to a given addrinfo. This socket will await a
    * client connection
    * 
    * @param listenSocketToSetup The file descriptor of the opened listen 
    * socket to bind to the addrinfo
    * @param listenSocketAddrInfo The addrinfo to bind the listen socket to
    * 
    * @return True if bind was succesful and false otherwise
    */
    bool BindListenSocket(int listenSocketToSetup, 
        addrinfo* listenSocketAddrInfo);
    
    /** 
    * Awaits client connection. This will stall the thread until a client
    * connects to this physics socket server.
    * 
    * @param listenSocket The listen socket to await connection
    * 
    * @return The connected client's socket for communication and -1 if any
    * error has occured while connecting to client
    */
    int AwaitClientConnection(int listenSocket);

    /** 
    * Awaits the receival of a client's message. This will call the socket's 
    * "recv" method to await any client's message.
    * 
    * @param clientSocket The client's connected socket to await messages on
    * @param receivingBuffer The buffer to store the received client's messages
    * @param receivingBufferLength The buffer's length to received the client's
    * messages
    * 
    * @return The number of bytes received on the client's message.
    */
    ssize_t ReceiveMessageFromClient(int clientSocket, char* receivingBuffer, 
        int receivingBufferLength);
    
    /** 
    * Sends a message to the client. 
    * 
    * @param clientSocket The connected client's socket to send the message to
    * @param messageBuffer The message to send the client
    * 
    * @return True if could succesfully send the message to the client and
    * false otherwise
    */
    bool SendMessageToClient(int clientSocket, const char* messageBuffer);

    /** Saves the step physics measurement to a file. */
    void SaveStepPhysicsMeasureToFile();

public:
    /** 
    * Initializes a physics system. On the param, could receive initial bodies
    * to create on the physics system.
    * 
    * The message's template should be:
    * "Init;\n
    * Id_0; posX_0; posY_0; posZ_0\n
    * Id_1; posX_1; posY_1; posZ_1\n
    * Id_2; posX_2; posY_2; posZ_2\n
    * ...
    * MessageEnd"
    */
    void InitializePhysicsSystem(const std::string initializationActorsInfo);

    /** 
    * Steps the current physics system simulation.
    * 
    * The message's template should be:
    * "Step"
    * 
    * @return The step physics simulation result. This will send each actor's
    * Id, position and rotation of the current physics system state back to
    * the client
    */
    std::string StepPhysicsSimulation();

public:
    /** 
    * Adds a new sphere body to the physics system.
    * The message template should be:
    * 
    * "AddSphereBody\n
    *  id; posX; posY; posZ; rotX; rotY; rotZ"
    * 
    * @param decodedMessageWithNewBodyInfo The received message from the client
    * with the info to create a new sphere body
    * 
    * @return The result of adding a new sphere body. May return a failure 
    * message if could not succesfully add the new sphere body on the physics 
    * system
    */
    std::string AddNewSphereBody
        (const std::string decodedMessageWithNewBodyInfo);

    /** 
    * Removes a body from the physics system.
    * The message template should be:
    * 
    * "RemoveBody\n
    * id"
    * 
    * @param decodedMessageWithRemoveBodyInfo The received message from the 
    * client with the info to remove the body
    * 
    * @return The result of removing a body. May return a failure message if 
    * could not succesfully remove the body from the physics system
    */
    std::string RemoveBody
        (const std::string decodedMessageWithRemoveBodyInfo);

private:
    /** 
    * The physics system implementation. This implement the JoltPhysics that
    * will initialize and update a physics world for this server
    */
    PhysicsServiceImpl* physicsServiceImplementation = nullptr;

    /** 
    *
    */
    class MessageHandlerParser* physicsServiceMessageHandlerParser = nullptr;

    /** 
    * The current physics step time measure without communication overhead.
    * Used to test the overall system
    */
	std::string currentPhysicsStepSimulationWithoutCommsTimeMeasure = "";

    /** 
    * The current decoded message. This is the current message received from
    * the client connected to this server.
    */
    std::string decodedMessage = "";
};

#endif
