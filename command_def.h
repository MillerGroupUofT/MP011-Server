#ifndef COMMAND_DEF_H
#define COMMAND_DEF_H

#define VERSION "5.4 (August 2015)"

#define PING_ANSWER "I am here!"

#define GOOD_STATUS_BYTE 1

typedef enum SENDMETHOD {TERM_STD, HEADER_NOTERM} SENDMETHOD;

#define DEFAULT_SEND_METHOD TERM_STD

SOCKET establish(unsigned short portnum);

#define MAX_OPTIONS 14 // max number of options per command
#define MAX_PARAMS 100 // max number of parameters per command

// Enumeration of all server functions
enum {INVALID, GETVERSION, NUMBEROFDEVICES, PRODUCTSTRING, OPEN, CLOSE, CHECKCOM, SENDRCV, SENDRCV_HEX, PING, PINGCOM}; 

// Structure used for storing received server commands
typedef struct
{
	int commandName;
	char options[MAX_OPTIONS];
	char* params[MAX_PARAMS];
} Command;

#endif
