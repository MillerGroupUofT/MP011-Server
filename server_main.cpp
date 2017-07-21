
#include <stdlib.h>

#include "server_functions.h"
#include "command_def.h"

using namespace std;

int main(int argc, char* argv[])
{
	int portnum;
	SOCKET s;
	DWORD err;
    TCHAR lpBuffer[256]; 

    // Get current time
    time_t raw_time;
    struct tm * time_info;
    raw_time = time(NULL);
    time_info = localtime(&raw_time);

	// Get listening port number
	if(argc != 3)
	{
		cout << "Error (missing parameters) " << endl;
		cout << "Usage: " << argv[0] << " <Device Name>" << " <Port Number>\n" << endl;
		return 1;
	}
	else
	{
		// Print current time and server version
		cout << "\nVersion " << VERSION << endl;
   		printf("Server (%s) started on %s", argv[1], asctime(time_info));
		
		// Get given port number
		portnum = atoi(argv[2]);
	}

	WORD wVersionRequested;
	WSADATA wsaData;
	int status_err;
	
	// Using MAKEWORD macro, Winsock version request 2.2
	wVersionRequested = MAKEWORD(2,2);
	 
	// Initialize Windows Socket API
	status_err = WSAStartup(wVersionRequested, &wsaData);
	err = GetLastError();
	if(status_err != 0)
	{
		// Tell the user that we could not find a usable WinSock DLL.
		perror("The Winsock dll not found!\n");
		
		// Get standard error message from system
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,				  // It´s a system error
						NULL,                                     // No string to be formatted needed
						err,                                      // Put error code here
						MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT), // Do it in the standard language
						lpBuffer,                                 // Put the error message here
						255,                    			      // Number of bytes to store the message (STR_ELEMS(lpBuffer)-1)
						NULL);
					
		cout << "Error (establish): " << lpBuffer << endl;

		return 1;
	}
	else
	{
		// Winsock dll is found
		printf("WSA status: %s\n", wsaData.szSystemStatus);
	}
	  
	// Confirm that the WinSock DLL supports 2.2.
	// Note that if the DLL supports versions greater than 2.2
	// in addition to 2.2, it will still return 2.2 in wVersion
	// since that is the version we requested
	if(LOBYTE(wsaData.wVersion) != 2 ||	HIBYTE(wsaData.wVersion) != 2)
	{
		// Tell the user that we could not find a usable WinSock DLL. 
		printf("The dll does not support the Winsock version %u.%u!\n", LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion));
		WSACleanup();
		
		return 1; 
	}
	// The WinSock DLL is acceptable. Proceed.
	
	// Create socket
	s = establish(portnum);
	err = GetLastError();
	if(s == INVALID_SOCKET)
	{ 
		perror("Error establishing socket\n");
		
		// Get standard error message from system
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,				  // It´s a system error
						NULL,                                     // No string to be formatted needed
						err,                                      // Put error code here
						MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT), // Do it in the standard language
						lpBuffer,                                 // Put the error message here
						255,                    			      // Number of bytes to store the message (STR_ELEMS(lpBuffer)-1)
						NULL);
					
		cout << "Error (establish): " << lpBuffer << endl;
				
		return 1;
	} 

	// Print list of available server commands
	Send_CommandList(s);

	// Wait for clients
	SOCKET new_sock;
	int nTimeout = 1800000;  // timeout in 30 minute
	for(;;)
	{ 
		printf("\nWaiting for client to connect...\n");

		if(s == INVALID_SOCKET)
		{ 
			fprintf(stderr, "Error waiting for new connection!\n"); 
			return 1;
		}

		new_sock = accept(s, NULL, NULL);
		err = GetLastError();
		if(new_sock == INVALID_SOCKET)
		{ 
			// Get standard error message from system
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,				  // It´s a system error
							NULL,                                     // No string to be formatted needed
							err,                                      // Put error code here
							MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT), // Do it in the standard language
							lpBuffer,                                 // Put the error message here
							255,                    			      // Number of bytes to store the message (STR_ELEMS(lpBuffer)-1)
							NULL);
						
			cout << "Error (accept): " << lpBuffer << endl;
			
			return 1;
		}
		
		// Set timeout condition on socket
		status_err = setsockopt(new_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*) &nTimeout, sizeof(int));
		err = GetLastError();
		if(status_err != 0)
		{
			// Get standard error message from system
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,				  // It´s a system error
							NULL,                                     // No string to be formatted needed
							err,                                      // Put error code here
							MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT), // Do it in the standard language
							lpBuffer,                                 // Put the error message here
							255,                    			      // Number of bytes to store the message (STR_ELEMS(lpBuffer)-1)
							NULL);
						
			cout << "Error (setsockopt): " << lpBuffer << endl;
        
			return 1;
		}

		Process_Request(new_sock); 
		
		closesocket(new_sock);

		printf("Client disconnected!\n");
	}

	WSACleanup();

	return 0;
}
