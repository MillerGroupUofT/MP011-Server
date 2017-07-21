
#include <sstream>
#include "server_functions.h"
#include "command_def.h"

#define BUFFERSIZE 1024
#define		WP2_CONTROLLER_POLLUX	128
#define		WMU_ASYNCMSG		WM_USER+10

DWORD		dwSuccess;
DWORD		dwAxes;
DWORD		dwControllerMode;
HINSTANCE	hWp2CommDll;
char 		rotflag[4];
char		initflag[10];

static HANDLE hcomm;

using namespace std;

typedef DWORD (__stdcall *pInitController)(DWORD CMode,DWORD Axes,DWORD Port, DWORD Baudrate,HWND UserWin,UINT UserMsg,DWORD Mode);
typedef DWORD (__stdcall *pOpenController)();
typedef DWORD (__stdcall *pCloseController)();
typedef DWORD (__stdcall *pExecuteCommand)(LPSTR pCommand, DWORD Lines, LPSTR pData);

int CMD_GETVERSION(Command cmd, SOCKET s, int NumParams, int NumOptions)
{
	string msg;

	msg = VERSION;
	
	printf("Sending server version...\n");

	SndLine(s,msg.data(),msg.length(),DEFAULT_SEND_METHOD);

	return 0;
}

int CMD_PING(Command cmd, SOCKET s, int NumParams, int NumOptions)
{
	string msg;

	msg = PING_ANSWER;
	
	printf("Responding to ping...\n");

	SndLine(s,msg.data(),msg.length(),DEFAULT_SEND_METHOD);

	return 0;
}

int CMD_PINGCOM(Command cmd, SOCKET s, int NumParams, int NumOptions)
{
	string msg;

	msg = PING_ANSWER;
	
	printf("Responding to ping...\n");

	SndLine(s,msg.data(),msg.length(),DEFAULT_SEND_METHOD);

	return 0;
}

int CMD_OPEN(Command cmd, SOCKET s, int NumParams, int NumOptions)
{
	string msg;
	DWORD baud_rate=0;
	DCB dcb;
	COMMTIMEOUTS timeouts = {0};
	DWORD err;
	TCHAR lpBuffer[BUFFERSIZE];
	pInitController Wp2CommInitController;
	pOpenController Wp2CommOpenController;
	hWp2CommDll=0;
	dwAxes=1;
	dwControllerMode=WP2_CONTROLLER_POLLUX;
	baud_rate = (DWORD)atoi(cmd.params[1]);

    // cmd.params: COM# Baud_rate flow_control
    if(NumParams < 3)
    {
    	printf("Error (OPEN): Missing parameters\n");
    	msg = "COM opening error: Missing parameters";
    }
    else
	if(strcmp(cmd.params[3],"ROT") == 0)
	{
		strcpy(rotflag, "ROT");
		hWp2CommDll=LoadLibrary("WP2COMM.DLL");
		if (hWp2CommDll==NULL)
		{
			printf("Unable to load WP2COMM.DLL\n");
		}
		if (strcmp(initflag,"INITIATED") != 0)
		{
			Wp2CommInitController=(pInitController)GetProcAddress(hWp2CommDll,"InitController");
			if (Wp2CommInitController==0)
			{
				printf("GetProcAddress-Error (InitController)\n");
				msg = "RS-40 controller initialization error";
			}
			else
			{
				printf("Successfully initialized\n");
				msg = "RS-40 controller successfully initialized";
			}

			(void) SndLine(s,msg.data(),msg.length(),DEFAULT_SEND_METHOD);

			char port[] = " ";
			char *temp;
			int portnum;

			strncpy(port, cmd.params[0], 3);
			temp = strtok(cmd.params[0],port);
			portnum = atoi(temp);

		        dwSuccess=(*Wp2CommInitController)(dwControllerMode,dwAxes,portnum,baud_rate,0,0,1050884);
			strcpy(initflag,"INITIATED");
		}

		        Wp2CommOpenController=(pOpenController)GetProcAddress(hWp2CommDll,"OpenController");
		        dwSuccess=(*Wp2CommOpenController)();

		        if (dwSuccess)
		        {
	        	msg = "RS-40 COM port opening error";
		        }
		        else
	        	{
			msg = "RS-40 COM port successfully opened";
			err = 0;
		        }
	}
	else
	{
		// Fix for accessing COM port number higher than 9
		char port[] = "\\\\.\\";
		strcat(port, cmd.params[0]);
		printf("COM port:\t %s\n", port);

		// Get user-supplied Baud rate
		baud_rate = (DWORD)atoi(cmd.params[1]);
		printf("Baud rate:\t %lu\n", baud_rate);

		// Check validy of supplied flow control setting
		if((strcmp(cmd.params[2],"NONE") != 0) && (strcmp(cmd.params[2],"XON/XOFF") != 0) && (strcmp(cmd.params[2],"HARDWARE") != 0))
		{
			printf("Error (OPEN): Invalid flow control parameter\n");
	    		msg = "COM opening error: Invalid flow control parameter";
		}
		else
		{
			// Create COM port handle
			hcomm = CreateFile(port,
		                     GENERIC_READ | GENERIC_WRITE,
		                     0,
		                     0,
		                     OPEN_EXISTING,
		                     0,
		                     0);
			err = GetLastError();           
			

			// Get COM port properties
			COMMPROP commProp;
			if(GetCommProperties(hcomm, &commProp) == FALSE)
			{
				err = GetLastError(); 
				printf("GetCommProperties error code: %lu\n", err);
			}
			//printf("commprop.dwMaxBaud = %x\n", (int)commProp.dwMaxBaud);
			//printf("commprop.dwSettableBaud = %x\n", (int)commProp.dwSettableBaud);

			// Check validity of supplied Baud rate
			if((commProp.dwSettableBaud & baud_rate) != baud_rate)
			{
				printf("Warning (OPEN): Invalid Baud rate parameter\n");
				msg = "COM opening warning: Invalid Baud rate parameter";			
			}
			
			// Check handle status
			if(hcomm == INVALID_HANDLE_VALUE)
			{
				hcomm = NULL;
				printf("Handle creation error code: %lu\n", err);
				msg = "COM opening error";
			}
			else
			{
				// Set timeout settings
				timeouts.ReadIntervalTimeout         = 0;
				timeouts.ReadTotalTimeoutMultiplier  = 0;
				timeouts.ReadTotalTimeoutConstant    = 100;
				timeouts.WriteTotalTimeoutMultiplier = 0;
				timeouts.WriteTotalTimeoutConstant   = 0;

				if(SetCommTimeouts(hcomm,&timeouts) == FALSE)
				{
					err = GetLastError(); 
					printf("Timeout setting error code: %lu\n", err);
					msg = "COM opening error";
				}
				else
				{
					// Initialize DCB
					FillMemory(&dcb,sizeof(dcb),0);
					if(GetCommState(hcomm,&dcb) == FALSE)
					{     
						err = GetLastError(); // Error in GetCommState
						printf("DCB setting error code: %lu\n", err);
						msg = "COM opening error";
					}
					
					// Setup DCB setting
					dcb.DCBlength = sizeof(DCB);
					dcb.BaudRate = baud_rate;
					dcb.Parity = NOPARITY;	
					dcb.ByteSize = 8;
					dcb.StopBits = ONESTOPBIT;
					dcb.fParity = FALSE;

					// DCB settings for flow control
					printf("Flow control:\t %s\n",cmd.params[2]);
					if(strcmp(cmd.params[2],"XON/XOFF") == 0)
					{
						dcb.fOutX = TRUE;						// Indicates whether XON/XOFF flow control is used during transmission
						dcb.fInX = TRUE;						// Indicates whether XON/XOFF flow control is used during reception
						dcb.fOutxCtsFlow = FALSE;				// If TRUE, the CTS (clear-to-send) signal is monitored for output flow control
						dcb.fOutxDsrFlow = FALSE;				// If TRUE, the DSR (data-set-ready) signal is monitored for output flow control
						dcb.fDsrSensitivity = FALSE;			// If TRUE, the communications driver is sensitive to the state of the DSR signal
						dcb.fRtsControl = RTS_CONTROL_DISABLE;	// RTS (request-to-send) flow control
						dcb.fDtrControl = DTR_CONTROL_DISABLE;	// DTR (data-terminal-ready) flow control
					}
					else if(strcmp(cmd.params[2],"HARDWARE") == 0)
					{
						dcb.fOutX = FALSE;
						dcb.fInX = FALSE;
						dcb.fOutxCtsFlow = TRUE;
						dcb.fOutxDsrFlow = TRUE;
						dcb.fDsrSensitivity = TRUE;
						dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
						dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
					}
					else if(strcmp(cmd.params[2],"NONE") == 0)
					{

					}

					// Set device control settings
					if(SetCommState(hcomm, &dcb) == FALSE)
					{
						err = GetLastError(); 
						printf("DCB setting error code: %lu\n", err);
						msg = "COM opening error";
					}
					else
					{
						msg = "COM successfully opened";
					}
				}
			}

		}
	}

	// Get standard error message from system
	if(err != 0)
	{
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,					// It큦 a system error
						NULL,										// No string to be formatted needed
						err,										// Put error code here
						MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),	// Do it in the standard language
						lpBuffer,									// Put the error message here
						BUFFERSIZE-1,								// Number of bytes to store the message (STR_ELEMS(lpBuffer)-1)
						NULL);
						
		cout << "Error (OPEN): " << lpBuffer << endl;
	}
	else
	{
		cout << msg << endl;
	}
	
	// Send result back to client
	//int i = SndLine(s,msg.data(),msg.length(),DEFAULT_SEND_METHOD);
	(void) SndLine(s,msg.data(),msg.length(),DEFAULT_SEND_METHOD);

	return 0;
}


int CMD_CLOSE(Command cmd,SOCKET s,int NumParams,int NumOptions)
{
	string msg;
	BOOL status_closeCOM = FALSE;
	DWORD err;
	TCHAR lpBuffer[BUFFERSIZE];
	pCloseController                Wp2CommCloseController;

	if(strcmp(rotflag,"ROT")==0)
	{
		Wp2CommCloseController=(pCloseController)GetProcAddress(hWp2CommDll,"CloseController");
		dwSuccess=(*Wp2CommCloseController)();

		if (dwSuccess)
		{
			msg = "RS-40 COM port closing error";
		}
		else
		{
			msg = "RS-40 COM port successfully closed";
		}
	}
	else
	{
		status_closeCOM = CloseHandle(hcomm);
		err = GetLastError();
	
		// Check status
		if(status_closeCOM == FALSE)
		{
			msg = "COM closing error";
		
		// Get standard error message from system
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,				  // It큦 a system error
							NULL,                                     // No string to be formatted needed
							err,                                      // Put error code here
							MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT), // Do it in the standard language
							lpBuffer,                                 // Put the error message here
							BUFFERSIZE-1,                    		  // Number of bytes to store the message (STR_ELEMS(lpBuffer)-1)
							NULL);
						
//		cout << "Error (CloseHandle): " << lpBuffer << endl;
		}
		else
		{
			msg = "COM successfully closed";
		}
	}
	cout << msg << endl;

	SndLine(s,msg.data(),msg.length(),DEFAULT_SEND_METHOD);

	rotflag[0]='\0';

	return 0;
}


int CMD_CHECKCOM(Command cmd,SOCKET s,int NumParams,int NumOptions)
{
	DCB dcb;
	string msg;
	DWORD err;
	TCHAR lpBuffer[BUFFERSIZE]; 
	BOOL status_checkCOM = FALSE;
    
	status_checkCOM = GetCommState(hcomm,&dcb);
	err = GetLastError();
    
    if(status_checkCOM == FALSE)
    {
        msg = "COM handle error";
        
        // Get standard error message from system
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,				  // It큦 a system error
						NULL,                                     // No string to be formatted needed
						err,                                      // Put error code here
						MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT), // Do it in the standard language
						lpBuffer,                                 // Put the error message here
						BUFFERSIZE-1,                    		  // Number of bytes to store the message (STR_ELEMS(lpBuffer)-1)
						NULL);
						
		cout << "Error (GetCommState): " << lpBuffer << endl;
	}
	else
	{
		msg = "COM handle OK";
	}
	cout << "COM checking status: " << msg << endl;
    
	//int i = SndLine(s,msg.data(),msg.length(),DEFAULT_SEND_METHOD);
	(void) SndLine(s,msg.data(),msg.length(),DEFAULT_SEND_METHOD);

	return 0;
}

int CMD_SENDRCV(Command cmd, SOCKET s, int NumParams, int NumOptions)
{
	int i = 0;
	string msg;
	
	char WriteBuffer[BUFFERSIZE];
	char ReadBuffer[BUFFERSIZE] = {0};	// BUFFERSIZE = 1024
	BOOL status_writeCOM = FALSE;
	BOOL status_readCOM = FALSE;
	DWORD dwBytesToWrite = 0;
	DWORD dwBytesWritten = 0;
	DWORD dwBytesRead = 0;
	DWORD err;
	TCHAR lpBuffer[BUFFERSIZE];
	pExecuteCommand         Wp2CommExecuteCommand;


	// Construct command message for device
	strcpy(WriteBuffer, cmd.params[1]);
	if(NumParams > 2)
	{
		for(i = 2; i < NumParams; i++)
		{
			strcat(WriteBuffer,cmd.params[i]);
		}
	}

	// Get termination character(s) to be appended
	char term_char[BUFFERSIZE];
	strcpy(term_char, cmd.params[0]);

	// Chop off EOL
	char *cp;
	if ((cp = strchr(term_char, '\r')) != 0)
	{
		*cp = 0;
	}
	if ((cp = strchr(term_char, '\n')) != 0)
	{
		*cp = 0;
	}
	if ((cp = strchr(term_char, ' ')) != 0)
	{
		*cp = 0;
	}


	// Put termination character(s) at end of message
	if(strcmp(term_char, "CRLF") == 0)
	{
		strcat(WriteBuffer, "\r\n");
	}
	else if(strcmp(term_char, "LFCR") == 0)
	{
		strcat(WriteBuffer, "\n\r");
	}
	else if(strcmp(term_char, "LF") == 0)
	{
		strcat(WriteBuffer, "\n");
	}
	else if(strcmp(term_char, "CR") == 0)
	{
		strcat(WriteBuffer, "\r");
	}
	else if(strcmp(term_char, "NONE") == 0)
	{
		// Do nothing
	}
	else
	{
		printf("Error (SENDRCV): Invalid termination character(s)\n");
	    	msg = "SENDRCV error: Invalid termination character(s)";

	    	(void) SndLine(s,msg.data(),msg.length(),DEFAULT_SEND_METHOD);

	    	return -1;
	}

	dwBytesToWrite = (DWORD) strlen(WriteBuffer);
	printf("Command (size: %lu bytes): %s\n", dwBytesToWrite, WriteBuffer);
	if(strcmp(rotflag,"ROT")==0)
	{
		Wp2CommExecuteCommand=(pExecuteCommand)GetProcAddress(hWp2CommDll,"ExecuteCommand");
		dwSuccess=(*Wp2CommExecuteCommand)(strlwr(WriteBuffer),1,ReadBuffer);
		msg = ReadBuffer;
		dwBytesRead = (DWORD) strlen(ReadBuffer);
	}
	else
	{
	
	// Send command to COM port
	status_writeCOM = WriteFile(hcomm,				// open file handle
					WriteBuffer,		// start of data to write
					dwBytesToWrite,		// number of bytes to write
					&dwBytesWritten,	// number of bytes that were written
					NULL);				// no overlapped structure
								
	// Error check
	err = GetLastError();
	if(status_writeCOM == FALSE)
	{
        	msg = "COM WriteFile error";
        
        	// Get standard error message from system
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,				  // It큦 a system error
						NULL,                                     // No string to be formatted needed
						err,                                      // Put error code here
						MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT), // Do it in the standard language
						lpBuffer,                                 // Put the error message here
						BUFFERSIZE-1,                    		  // Number of bytes to store the message (STR_ELEMS(lpBuffer)-1)
						NULL);
						
		cout << "Error (WriteFile): " << lpBuffer << endl;
		
		// Send device message to client
		i = SndLine(s,msg.data(),msg.length(),DEFAULT_SEND_METHOD);
		
		return -1;
	}
	else if(dwBytesWritten != dwBytesToWrite)
    	{
		// This is an error because a synchronous write that results in
		// success (WriteFile returns TRUE) should write all data as
		// requested. This would not necessarily be the case for
		// asynchronous writes.
		printf("Error: dwBytesWritten != dwBytesToWrite\n");
            
		// Send device message to client
		msg = "COM WriteFile error";
		i = SndLine(s,msg.data(),msg.length(),DEFAULT_SEND_METHOD);
            
		return -1;
	}
	else
	{
		printf("Wrote %lu bytes successfully\n", dwBytesWritten);
	}

	// Wait for device processing
	Sleep(350);

	// Receive message from COM port
	status_readCOM = ReadFile(hcomm,          // open file handle
                              ReadBuffer,   // start of data to be read
                              BUFFERSIZE,   // number of bytes to read
                              &dwBytesRead,   // number of bytes that were read
                              NULL);          // no overlapped structure
	// Error check
	err = GetLastError();
	if(status_readCOM == FALSE)
	{        
		// Get standard error message from system
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,                // It큦 a system error
                      NULL,                                      // No string to be formatted needed
                      err,                                       // Put error code here
                      MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),  // Do it in the standard language
                      lpBuffer,                                  // Put the error message here
                      BUFFERSIZE-1,                    		     // Number of bytes to store the message (STR_ELEMS(lpBuffer)-1)
                      NULL);
						
//		cout << "Error (ReadFile): " << lpBuffer << endl;
		
		msg = "COM ReadFile error";
		i = SndLine(s,msg.data(),msg.length(),DEFAULT_SEND_METHOD);
		
		return -1;
	}
	}

	// Assuming the read data is ANSI text	
	if((dwBytesRead > 0) && (dwBytesRead <= BUFFERSIZE))
	{
        	ReadBuffer[dwBytesRead+1]='\0'; // NULL character
		printf("Data read (%lu bytes): %s\n",dwBytesRead,ReadBuffer);
		
		// Send device message to client
		ReadBuffer[dwBytesRead+1] = '\0';
		msg = ReadBuffer;
		i = SndLine(s,msg.data(),msg.length(),DEFAULT_SEND_METHOD);
	}
	else if(dwBytesRead == 0)
	{	
		printf("No data read from COM port\n");
		msg = "No data read from COM port";
		i = SndLine(s,msg.data(),msg.length(),DEFAULT_SEND_METHOD);
	}
	else
	{
		msg = "COM ReadFile error";
		printf("Error (ReadFile): Unexpected value for dwBytesRead\n");
	}
	return 0;
}

int CMD_SENDRCV_HEX(Command cmd, SOCKET s, int NumParams, int NumOptions)
{
	int i = 0;
	string msg;
	
	unsigned char WriteBuffer[BUFFERSIZE] = {0};	
	unsigned char ReadBuffer[BUFFERSIZE] = {0};	// BUFFERSIZE = 256
	BOOL status_writeCOM = FALSE;
	BOOL status_readCOM = FALSE;
	DWORD dwBytesToWrite = 0;
	DWORD dwBytesWritten = 0;
	DWORD dwBytesRead = 0;
	DWORD err;
	TCHAR lpBuffer[BUFFERSIZE];

    // Remove trailing whitespaces
	char *cp;
	for(i = 0; i < NumParams; i++)
	{
		if ((cp = strchr(cmd.params[i], '\r')) != 0)
		{
			*cp = 0;
		}
		if ((cp = strchr(cmd.params[i], '\n')) != 0)
		{
			*cp = 0;
		}
		if ((cp = strchr(cmd.params[i], ' ')) != 0)
		{
			*cp = 0;
		}
	}

	// Construct command message for device
	if(NumParams > 2)
	{
		for(i = 0; i < NumParams; i++)
		{
			WriteBuffer[i] = strtoul(cmd.params[i], NULL, 16);
		}
	}

	// DEBUG
	printf("NumParams = %d\n", NumParams);
	for(i = 0; i < NumParams ; i++)
	{
		printf("WriteBuffer %d = %s \n", i, cmd.params[i]);
	}

	dwBytesToWrite = (DWORD) (NumParams);
	printf("Command (size: %lu bytes): ", dwBytesToWrite);
	for(i = 0; i < (NumParams); i++)
	{
		printf("%X (%s)", WriteBuffer[i], cmd.params[i]);
	}
	printf("\n");

	
	// Send command to COM port
	status_writeCOM = WriteFile(hcomm,	// open file handle
			WriteBuffer,		// start of data to write
			dwBytesToWrite,		// number of bytes to write
			&dwBytesWritten,	// number of bytes that were written
			NULL);			// no overlapped structure
								
	// Error check
	err = GetLastError();
    if(status_writeCOM == FALSE)
    {
        msg = "COM WriteFile error";
        
        // Get standard error message from system
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,		  // It큦 a system error
			NULL,                                     // No string to be formatted needed
			err,                                      // Put error code here
			MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT), // Do it in the standard language
			lpBuffer,                                 // Put the error message here
			BUFFERSIZE-1,                  		  // Number of bytes to store the message (STR_ELEMS(lpBuffer)-1)
			NULL);
						
		cout << "Error (WriteFile): " << lpBuffer << endl;
		
		// Send device message to client
		i = SndLine(s,msg.data(),msg.length(),DEFAULT_SEND_METHOD);
		
		return -1;
	}
	else if(dwBytesWritten != dwBytesToWrite)
    {
		// This is an error because a synchronous write that results in
		// success (WriteFile returns TRUE) should write all data as
		// requested. This would not necessarily be the case for
		// asynchronous writes.
		printf("Error: dwBytesWritten != dwBytesToWrite\n");
            
		// Send device message to client
		msg = "COM WriteFile error";
		i = SndLine(s,msg.data(),msg.length(),DEFAULT_SEND_METHOD);
            
		return -1;
	}
    else
    {
			printf("Wrote %lu bytes successfully\n", dwBytesWritten);
    }

	// Wait for device processing
	Sleep(350);

	// Receive message from COM port
	status_readCOM = ReadFile(hcomm,          // open file handle
                              ReadBuffer,   // start of data to be read
                              BUFFERSIZE,   // number of bytes to read
                              &dwBytesRead,   // number of bytes that were read
                              NULL);          // no overlapped structure
	// Error check
	err = GetLastError();
	if(status_readCOM == FALSE)
	{        
		// Get standard error message from system
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,       // It큦 a system error
                      NULL,                                     // No string to be formatted needed
                      err,                                      // Put error code here
                      MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT), // Do it in the standard language
                      lpBuffer,                                 // Put the error message here
                      BUFFERSIZE-1,				// Number of bytes to store the message (STR_ELEMS(lpBuffer)-1)
                      NULL);
						
//		cout << "Error (ReadFile): " << lpBuffer << endl;
		
		msg = "COM ReadFile error";
		i = SndLine(s,msg.data(),msg.length(),DEFAULT_SEND_METHOD);
		
		return -1;
	}
		
	// Assuming the read data is ANSI text	
	if((dwBytesRead > 0) && (dwBytesRead <= BUFFERSIZE))
    {
	    unsigned int m;
	    char answer[40]={'\0'};
	    char temp[3];
	    temp[0] = '\0';
	    ReadBuffer[dwBytesRead+1]='\0'; // NULL character
		printf("Data read (%lu bytes)\n",dwBytesRead);
		for(m = 0; m < dwBytesRead; m++)
		{
			printf("ReadBuffer %02X\n", ReadBuffer[m]);
			sprintf(temp, "%02X", ReadBuffer[m]);
			strcat(answer, temp);
		}
		
		// Send device message to client
		ReadBuffer[dwBytesRead+1] = '\0';
		printf("Answer: %s\n", answer);
		msg = answer;
		i = SndLine(s,msg.data(),msg.length(),DEFAULT_SEND_METHOD);
    }
	else if(dwBytesRead == 0)
	{	
		printf("No data read from COM port\n");
		msg = "No data read from COM port";
		i = SndLine(s,msg.data(),msg.length(),DEFAULT_SEND_METHOD);
	}
	else
	{
		msg = "COM ReadFile error";
		printf("Error (ReadFile): Unexpected value for dwBytesRead\n");
	}

	return 0;
}
