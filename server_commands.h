
/*
* server_commands.h
*
* Defines a set of server functions that correspond to user commands. The purpose of these functions is to take a 
* user command and parameters and call the corresponding instrument specific function.
*/

#ifndef _SERVER_COMMANDS_H
#define _SERVER_COMMANDS_H

#include "server_functions.h"
#include "command_def.h"

using namespace std;

// Get server version
int CMD_GETVERSION(Command cmd, SOCKET s, int NumParams, int NumOptions);

// Reply to ping
int CMD_PING(Command cmd, SOCKET s, int NumParams, int NumOptions);

// Ping COM port with client-provided command
int CMD_PINGCOM(Command cmd, SOCKET s, int NumParams, int NumOptions);

// Open COM port
int CMD_OPEN(Command cmd, SOCKET s, int NumParams, int NumOptions);

// Close COM port
int CMD_CLOSE(Command cmd, SOCKET s, int NumParams, int NumOptions);

// Check status of COM port handle
int CMD_CHECKCOM(Command cmd, SOCKET s, int NumParams, int NumOptions);

// Send and receive data from COM port
int CMD_SENDRCV(Command cmd, SOCKET s, int NumParams, int NumOptions);

// Send and receive data from COM port (convert to char to hexadecimal)
int CMD_SENDRCV_HEX(Command cmd, SOCKET s, int NumParams, int NumOptions);

#endif
