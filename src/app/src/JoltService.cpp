#include "Communication/PhysicsServiceSocketServer.h"

int main(int argc, char** argv) 
{
    // Open socket acting as a server socket
    // The proxy will await for the game's connection on him
    PhysicsServiceSocketServer* PhysicsServiceServer = 
        new PhysicsServiceSocketServer();
    if(!PhysicsServiceServer)
    {
        printf("Error when creating socket server.\n");
        return 0;
    }
    
    // Check if there are additional command arguments
    if(argc > 1)
    {
        // Get the first command arg
        const auto firstCommandArg = argv[1];

        // Check if command is "nosocket"
        if(strcmp(firstCommandArg, "nosocket") == 0)
        {
            // If it is, just run a debug simulation, instead of opening a
            // socket
            PhysicsServiceServer->RunDebugSimulation();

            return 0;
        }

        // Else, the first command should be the server port
        // Open server socket to listen for client's (game) connection
        const bool bWasSocketConnectionSuccess = 
            PhysicsServiceServer->OpenServerSocket(firstCommandArg);

        // Check for errors
        if(!bWasSocketConnectionSuccess)
        {
            printf("Could not open socket connection. Check logs.\n");
            return 0;
        }

        return 0;
    }

    std::cout 
        << "The command should have at least one argument. Either the server's port or \"nosocket\"";

    return 0;
}
