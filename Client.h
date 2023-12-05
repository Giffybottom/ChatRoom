#pragma once
#include "platform.h"

class Client
{
public:
	Client();
	~Client();

	SOCKET listenSocket;
	SOCKET UDPClient;
	SOCKET ComSocket;

	u_short ClientPort;

	char broadcast = 1;
	char userUsername[100];
	char IPAddress[20];

	std::ofstream toClientLog_OStream;

void readMessage(SOCKET s, char* buffer, int32_t size);
void sendMessage(char* data, int32_t length);

bool initClient();

bool updateClient();

void closeClient(SOCKET s);

private:
	
	bool ClientCommandCheck(char* buffer);
	bool serverFull;
	char serverMessage[3];

	std::thread sendThread;
	std::thread recieveThread;

	std::mutex threadProtection;

	char sendBuffer[256];
	char inBuffer[256];
	u_long blockValue;

	bool continueToRun = true;
	bool loggingStuff = false;
};
