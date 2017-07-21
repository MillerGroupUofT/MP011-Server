
/*
* server_functions
*/
#include "server_functions.h"
#include "command_def.h"

#define MAXBUFFER 1024

using namespace std;

int Tokenize(const char* const s,int len,int n,char* r,const char t)
{
	
	int i,j,k,count = 0; //count is # of tokens (not counting contiguous tokens) seen
	int left = -1; 		 //left is start of token
	int rlen = 0;        //rlen is the right position of the token
	
	//printf("strlen is %d, string is %s\n",len,s);

	for(i = 0; i < len; i++)
	{
		//printf("char %c\n",s[i]);
		if(s[i] == t || i == 0)
		{
			//skip for contiguous tokens
			k = i+1;
			while(k < len && s[k++] == t)
			{
				i++;
			}
		
			if(count < (n-1))
			{
				count++; //keep skipping tokens until we get to the one we want to extract
			}
			else if(count == (n-1))
			{
				left = i;
				for(j = i+1; j < len; j++, rlen++)
				{
					//grab the token
					//printf("%c\n",s[j]);
					if(s[j] == t)
						break;
				}
				break;
			}
		}
	}

	if(left == -1)
	{
		return 0;
	}
	rlen++;

	if(i != 0)
	{
		left++;
	}
	
	memcpy(r, s + left, rlen);
	r[rlen] = 0;
	
	return rlen;
}

SOCKET establish(unsigned short portnum)
{ 
	char myname[256]; 
	SOCKET s; 
	struct sockaddr_in sa; 
	struct hostent *hp; 

	// clear our address
	memset(&sa, 0, sizeof(struct sockaddr_in)); 

	// who are we?
	gethostname(myname, sizeof(myname)); 
	
	// get our address info
	hp = gethostbyname(myname); 

	// we don't exist !?
	if (hp == NULL) 
		return(INVALID_SOCKET); 

	// this is our host address
	sa.sin_family = hp->h_addrtype; 

	// this is our port number
	sa.sin_port = htons(portnum); 

	// create the socket
	s = socket(AF_INET, SOCK_STREAM, 0); 

	if (s == INVALID_SOCKET) 
		return INVALID_SOCKET;

	// bind the socket to the internet address
	if (bind(s, (struct sockaddr *)&sa, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
	{ 
		closesocket(s); 
		return(INVALID_SOCKET); 
	} 

	// max number of queued connects
	listen(s, 0);

	return(s); 
}

int SndLine(SOCKET s,const char* buff,int len,SENDMETHOD m)
{
	int numbytes = 0;
	DWORD err;

	if(m == TERM_STD)
	{
		numbytes = send(s,buff,len,0);
		err = GetLastError();
		
		if(numbytes != len)
		{
			printf("Line sending error code: %lu\n", err);	
			perror("Error occurred while attempting to send line through socket\n");
			return -1;
		}
		
		// Termination characters
		char term[2];
		term[0] = '\r'; term[1] = '\n'; // CRLF
		//term[0] = '\n'; term[1] = '\r'; // LFCR		
		numbytes += send(s,term,2,0);
		if(numbytes != len+2)
		{
			printf("Line termination error code: %lu\n", err);	
			perror("Error occurred while attempting to send line through socket\n");
			return -1;
		}

		printf("Sent %d bytes of data\n", numbytes);
	}

	if(m == HEADER_NOTERM)
	{
		int count = len;
		if((numbytes = send(s,(char*)&count,sizeof(int),0)) != sizeof(int))
		{
			perror("Error occurred while attempting to send header through socket\n");
			return -1;
		}
		if((numbytes = send(s,buff,len,0) )!= len)
		{
			perror("Error occurred while attempting to send line through socket\n");
			return -1;
		}
		printf("Sent %d bytes of data\n",numbytes+sizeof(int));
	}
	
	return numbytes;
}

int SndData(SOCKET s,const char* buff,int len,unsigned short xsize,unsigned short ysize)
{
	int numbytes;
	unsigned short x = htons(xsize);
	unsigned short y = htons(ysize);
	numbytes = 0;
	
	//printf("%d %d\n",x,y);
	//char my_array[] = "abcdefg";

	if(len <= 0)
	{
		numbytes = 0;
		send(s, (char*)&numbytes, sizeof(int),0);
		return 0;
	}
	
	printf("xy %d %d\n", x, y);

	if((numbytes = send(s,(char*)&x, sizeof(unsigned short),0)) != sizeof(unsigned short))
	{
		perror("Error(1) occurred while attempting to send header through socket\n");
		return -1;
	}

	if((numbytes = send(s,(char*)&y, sizeof(unsigned short),0)) != sizeof(unsigned short))
	{
		perror("Error(2) occurred while attempting to send header through socket\n");
		return -1;
	}

	/*
	if( (numbytes = send(s,(char*)&my_array,8,0) )!=8)
	{
		perror("Error(3) occurred while attempting to send data through socket\n");
		return -1;
	}
	*/

	if((numbytes = send(s,buff,len,0)) != len)
	{
		perror("Error(3) occurred while attempting to send data through socket\n");
		return -1;
	}

	printf("Sent %d bytes of data\n", len+2*sizeof(unsigned short));

	return (numbytes+2*sizeof(unsigned short));
}

int SndByte(SOCKET s, const char c)
{
	char byte = c;
	int numbytes;
	if( (numbytes = send(s,&byte,sizeof(char),0)) !=sizeof(char))
	{
		perror("Error occurred while attempting to send byte through socket\n");
		return -1;
	}
	return 0;
}

int RcvLine(SOCKET s, char* buff)
{

	int curcount = 0;
	int totalcount = 0;

	DWORD err;
    TCHAR lpBuffer[256];

	do
	{
        curcount = recv(s, buff+totalcount, MAXBUFFER, 0);
        
        err = GetLastError();
		if(err != 0)
		{
			// Get standard error message from system
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,				  // It´s a system error
							NULL,                                     // No string to be formatted needed
							err,                                      // Put error code here
							MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT), // Do it in the standard language
							lpBuffer,                                 // Put the error message here
							255,                    		          // Number of bytes to store the message (STR_ELEMS(lpBuffer)-1)
							NULL);
							
			cout << "Error (RECV): " << lpBuffer << endl;
			
			return -1;
		}
        
		if(curcount > 0)
		{
			totalcount += curcount;
			if(buff[totalcount - 2] == '\r' && buff[totalcount - 1] == '\n')
				// newline, end transmission
				break;
		}
		else if(curcount == 0)
		{
            printf("Connection closed\n");
			return 0;
		}
		else
		{
            printf("recv failed: %d\n", WSAGetLastError());
			return -1;
		}

    }while(curcount > 0);

	buff[totalcount] = 0;

	return totalcount;
}

void Send_CommandList(SOCKET s)
{

	string msg;

	printf("\nAvailable server commands:\n");

	printf("GETVERSION\n");

	printf("\tDescription: Get version ID of server program\n");

	printf("\tSynopsis: GETVERSION\n");


	printf("PING\n");
	
	printf("\tDescription: Get server reply (\"I am here!\")\n");
	
	printf("\tSynopsis: PING\n");
	

	printf("OPEN\n");
	
	printf("\tDescription: Open serial communications port\n");
	
	printf("\tSynopsis: OPEN COM%%d Baud_rate flow_control\n");
	
	printf("\tParameters:\n");
	
	printf("\t\t- string COM%%d, port number (where %%d is an integer > 0)\n");
	
	printf("\t\t- int Baud_rate, Baud rate (e.g. 9600, 19200, 57600)\n");
	
	printf("\t\t- string flow_control, flow control setting (\"none\",\"Xon/Xoff\",\"hardware\")\n");
	

	printf("CLOSE\n");
	
	printf("\tDescription: Close previously opened COM port\n");
	
	printf("\tSynopsis: CLOSE\n");
	

	printf("CHECKCOM\n");
	
	printf("\tDescription: Check status of COM port\n");
	
	printf("\tSynopsis: CHECKCOM\n");
	

	printf("SENDRCV\n");
	
	printf("\tDescription: Send and receive data through open serial communications port\n");
	
	printf("\tSynopsis: SENDRCV term_char command\n");
	
	printf("\tParameters:\n");
	
	printf("\t\t- str term_char, termination character(s) to be appended to data (\"CRLF\", \"LFCR\", \"CR\", \"LF\")\n");
	
	printf("\t\t- str command, ASCII text to be sent through COM port\n");


	return;
}

void Process_Request(SOCKET s)
{
	string msg;
	char buff[MAXBUFFER];
	char* p; // Pointer to parameters

	printf("Client Connected!\n");
	msg = "Connected to server!";
	(void) SndLine(s,msg.data(),msg.length(),DEFAULT_SEND_METHOD);

	while(RcvLine(s,buff) > 0)
	{
		printf("\n\n");
		printf("Received raw packet: %s",buff);

		// Chop off EOL
		char *cp;
		if ((cp = strchr(buff, '\r')) != 0)
			*cp = 0;
		if ((cp = strchr(buff, '\n')) != 0)
			*cp = 0;

		// Check if "quit" command received in buffer
		printf("Command Received: %s\n",_strupr(buff));
		if(strcmp(_strupr(buff),"QUIT") == 0)
		{
			printf("Disconnecting Client...\n");

			msg = "Disconnected from server!";
			(void) SndLine(s,msg.data(),msg.length(),DEFAULT_SEND_METHOD);

			return;
		}

		char r[50];
		int TokenCnt = 1;
		int i = 0;
		int y = 0;
		int numParams = 0;
		int numOptions = 0;
		Command newCommand;
		newCommand.commandName = INVALID;
				
		while((y = Tokenize(buff,strlen(buff),TokenCnt,r,' ')) != 0)
		{
			// printf("the token is: %s\n",r);
			// token 1 = command
			if(TokenCnt == 1)
			
			// '-' = options
			if((TokenCnt > 1) && (r[0]=='_'))
			{	
				for(i = 0; i < (int) strlen(r) && i < MAX_OPTIONS;i++)
				{
					newCommand.options[i] = r[i];
					numOptions++;
				}
				
				break;
			}

			//rest of tokens = params
			if((TokenCnt > 1) && (numParams < MAX_PARAMS))
			{
				p = new char[strlen(r)+1];	//freed later
				strcpy(p,r);
				newCommand.params[numParams] = p;
				numParams++;
			}

			//printf("token count %d\n",TokenCnt);
			TokenCnt++;
		}

		if((y = Tokenize(buff,strlen(buff),1,r,' ')) != 0)
		{
			if(strcmp(_strupr(r),"GETVERSION") == 0)
			{
				newCommand.commandName = GETVERSION;
				CMD_GETVERSION(newCommand,s,numParams,numOptions);
			}
			else if(strcmp(_strupr(r),"PING") == 0)
			{
				newCommand.commandName = PING;
				CMD_PING(newCommand,s,numParams,numOptions);
			}
			else if(strcmp(_strupr(r),"PINGCOM") == 0)
			{
				newCommand.commandName = PINGCOM;
				CMD_PING(newCommand,s,numParams,numOptions);
			}
			else if(strcmp(_strupr(r),"OPEN") == 0)
			{
				newCommand.commandName = OPEN;
				CMD_OPEN(newCommand,s,numParams,numOptions);
			}
			else if(strcmp(_strupr(r),"CLOSE") == 0)
			{
				newCommand.commandName = CLOSE;
				CMD_CLOSE(newCommand,s,numParams,numOptions);
			}
			else if(strcmp(_strupr(r),"SENDRCV") == 0)
			{
				newCommand.commandName = SENDRCV;
				CMD_SENDRCV(newCommand,s,numParams,numOptions);
			}
			else if(strcmp(_strupr(r),"SENDRCV_HEX") == 0)
			{
				newCommand.commandName = SENDRCV_HEX;
				CMD_SENDRCV_HEX(newCommand,s,numParams,numOptions);
			}
			else if(strcmp(_strupr(r),"CHECKCOM") == 0)
			{
				newCommand.commandName = CHECKCOM;
				CMD_CHECKCOM(newCommand,s,numParams,numOptions);
			}
			// For more server commands, add here
			else
			{ 
				printf("Invalid Command!\n");
				msg = "Invalid Command!";
				(void) SndLine(s,msg.data(),msg.length(),DEFAULT_SEND_METHOD);
			}
		}

		for(i = 0; (i < (int) strlen(r)) && (i < MAX_OPTIONS); i++)
		{
			newCommand.options[i] = 0;
		}

		for(i = 0; i < numParams; i++)
		{
			delete[] newCommand.params[i];
		}

		for(i = 0; i < MAXBUFFER; i++)
		{
			buff[i] = 0;
		}
		
	}
	
	return;
}

