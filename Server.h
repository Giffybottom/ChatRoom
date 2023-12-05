#pragma once
#include "platform.h"

class Server
{
public:
	Server();
	~Server(); 

	SOCKET listenSocket;
	SOCKET UDPSender; // UDP
	SOCKET ComSocket;
	sockaddr_in UDPAddress; // UDP
	fd_set masterSet, readySet, deniedSet;
	u_short port = 31337;
	char broadcast = 1; // UDP
	char FullConnectionInfo[16] = "127.0.0.1!31337";

	std::string username = "";

	void readMessage(SOCKET s, char* buffer, int32_t size);
	int sendMessage(char* data, int32_t length);

	void initServer();
	void updateServer();
	void closeServer();
	
	
	//Server Failure Message Codes To Send to Clients
	char serverFullMessage[3] = {"00"}; //Server Full Message
	
	//Server Success Message Codes to Send to Clients
	char serverConnectMessage[3] = {"11"}; // Server Connected To
	
	std::map<SOCKET, std::string> registeredUsers;

	//FStream 
	//std::string serverLog = "";
	std::ofstream toServerLog_OStream;
	std::ifstream fromServerLog_IStream;
	
private:

	int SV_FULL = 3;
	int CURR_USER_COUNT = 0;

	bool ServerFullCheck();
	void sendMessageAllUsers(std::string data);
	void sendMessageSingleUser(SOCKET s, char* data, int32_t length);
	void sendMessageSingleUserString(SOCKET s, std::string message);
	std::string GetListOfRegisteredUsers();
};