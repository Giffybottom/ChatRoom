#include "Client.h"

Client::Client()
{
}

Client::~Client()
{
}

bool Client::initClient()
{
	int errorCheck;

	//Step 2
	ComSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	UDPClient = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (ComSocket == INVALID_SOCKET)
	{
		printf("DEBUG// TCP Socket function incorrect\n");
		return false;
	}
	else
	{
		printf("[TCP Socket Creation: Success]\n");
	}

	if (ComSocket == INVALID_SOCKET)
	{
		printf("DEBUG// UDP Socket function incorrect\n");
		return false;
	}
	else
	{
		printf("[UDP Socket Creation: Success]\n");
	}

	sockaddr_in udpAddr;
	udpAddr.sin_family = AF_INET;
	udpAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	udpAddr.sin_port = htons(31330);

	errorCheck = setsockopt(UDPClient, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

	if (errorCheck == SOCKET_ERROR)
	{
		printf("setsockopt failure\n");
	}
	else
	{
		printf("setsockopt success\n");
	}

	errorCheck = bind(UDPClient, (SOCKADDR*)&udpAddr, sizeof(udpAddr));

	if (errorCheck == SOCKET_ERROR)
	{
		printf("bind failure\n");
		errorCheck = WSAGetLastError();
	}
	else
	{
		printf("bind success\n");
	}

	char addressBuffer[20];
	int length = 20;
	int refLength = sizeof(udpAddr);

	errorCheck = recvfrom(UDPClient, addressBuffer, length, 0, (SOCKADDR*)&udpAddr, &refLength);

	if (errorCheck == SOCKET_ERROR)
	{
		printf("recvfrom failure\n");
		errorCheck = WSAEWOULDBLOCK;
	}
	else
	{
		printf("recvfrom success\n");
	}

	closesocket(UDPClient);

	std::string port = addressBuffer;
	std::string IP = port.substr(0, port.find_first_of('!'));
	port.erase(0, port.find_first_of('!') + 1);
	strcpy(IPAddress, IP.c_str());
	ClientPort = std::stoi(port);

	//Connect
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = inet_addr(IPAddress);
	serverAddr.sin_port = htons(ClientPort);

	int result = connect(ComSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (result == SOCKET_ERROR)
	{
		printf("DEBUG// Connect function incorrect\n");
		printf("Press enter to continue...");
		system("Pause");
		return false;
	}
	else
	{
		printf("[Connect: Successful]\n");
	}

	//Server messages prior to registration
	
	readMessage(ComSocket, serverMessage, 8);
	if (serverMessage[0] == '0' && serverMessage[1] == '0')
	{
		serverFull = true;
		printf("Server Message: Server Full\n");
		closeClient(ComSocket);
		return false;
	}
	else if (serverMessage[0] == '1' && serverMessage[1] == '1')
	{
		printf("[Chat Room Join: Successful]\n");
	}

	//Step 4
	printf("Your IP address is: %s\n", IPAddress);
	/*scanf_s("Your port number is: %i\n", ClientPort);*/
	printf("Your port number is: %i\n", ClientPort);
	std::cout << "What is your username?" << std::endl;
	std::cin.ignore();
	std::cin.get(userUsername, 100, '\n');
	std::cin.clear();

	//Set up register command
	std::string UsernameRegitry = "$Register<";
	UsernameRegitry.append(userUsername);
	UsernameRegitry.append(">");
	char* finalOutput = new char[UsernameRegitry.length() + 1];
	sendMessage(strncpy(finalOutput, UsernameRegitry.c_str(), (UsernameRegitry.length() + 1)), (UsernameRegitry.length() + 1));

	system("CLS");

	printf("Your IP address is: %s\n", IPAddress);
	/*scanf_s("Your port number is: %hu\n", ClientPort);*/
	printf("Your port number is: %hu\n", ClientPort);
	printf("Your username is: %s\n", userUsername);

	blockValue = 1;
	if (ioctlsocket(ComSocket, FIONBIO, &blockValue) == SOCKET_ERROR)
		return false;

	//lamda
	sendThread = std::thread([&] 
	{
		while (continueToRun)
		{
			char userMessage[255] = {};

			std::cin.ignore();
			std::cin.get(userMessage, 256, '\n');
			std::cin.clear();

			//locks thread so we can send a message
			std::lock_guard<std::mutex>lockMe(threadProtection);

			//clears buffer
			memset(sendBuffer, 0, sizeof(sendBuffer));

			//copying userMessage to the buffer
			strncpy_s(sendBuffer, userMessage, 255);

			sendMessage(sendBuffer, 255);

			//if message is a command
			if (userMessage[0] == '$')
			{
				//Parse command for validation
				std::string s_recievedMessage = userMessage;
				std::string commandCheck;

				//Identify Command
				commandCheck = s_recievedMessage.substr(1, s_recievedMessage.size());

				if (commandCheck == "Exit" || commandCheck == "exit")
				{
					continueToRun = false;
					closeClient(ComSocket);
				}
				else if (commandCheck == "Getlog" || commandCheck == "GetLog" || commandCheck == "getlog" || commandCheck == "getLog")
				{
					loggingStuff = true;
					std::cout << "Logging" << "\n";
				}
			}
		}
	});

	recieveThread = std::thread([&] 
	{
		while (continueToRun)
		{
			//locks thread so we can recieve message
			std::lock_guard<std::mutex>lockMe(threadProtection);

			//Clears inBuffer
			memset(inBuffer, '\0', sizeof(inBuffer));

			readMessage(ComSocket, inBuffer, 255);

			//'\0' = nullterminator
			if (inBuffer[0] != '\0')
			{
				if (!loggingStuff)
				{
					printf("%s", inBuffer);
				}
				else
				{
					std::string s_inBuffer = inBuffer;
					if(s_inBuffer == "SV_END_FILE")
					{
						loggingStuff = false;
						printf("Done Logging\n");
						continue;
					}
					toClientLog_OStream.open("ClientLog.txt", std::ios_base::app);

					if (!toClientLog_OStream.is_open())
					{
						std::cout << "Error: File did not open" << "\n";
						loggingStuff = false;
						continue;
					}

					

					toClientLog_OStream << s_inBuffer << '\n';

					toClientLog_OStream.close();
					//std::cout << "Done Logging" << "\n";

				}
			}
			
		}
	});

	if (sendThread.joinable())
	{
		sendThread.join();
	}

	if (recieveThread.joinable())
	{
		recieveThread.join();
	}
	return true;
}

bool Client::updateClient()
{
	return continueToRun;
}

void Client::closeClient(SOCKET s)
{
	shutdown(s, SD_BOTH);
	closesocket(s);
}

bool Client::ClientCommandCheck(char* buffer)
{
	//Parse for "$"
	//return true if found
	//return false if not
	
	if (buffer[0] == '$')
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Client::readMessage(SOCKET s, char* buffer, int32_t size)
{
	int result;
	int counterOfBytes = 0;
	int32_t buffersize = sizeof(buffer);
	if (sizeof(buffer) > size)
	{
		printf("Parameter Error");
		return;
	}
	result = tcp_recv_whole(s, (char*)&counterOfBytes, 1);
	if ((result == SOCKET_ERROR) || (result == 0))
	{
		result = WSAGetLastError(); //10035 non-block, nonfatal error, no resourse to grab
		//printf("Disconnect");
		return;

	}

	result = tcp_recv_whole(s, (char*)buffer, counterOfBytes);
	if ((result == SOCKET_ERROR) || (result == 0))
	{
		result = WSAGetLastError();
		//printf("Disconnect");
		return;
	}
	else
	{
		/*printf("[You have connected]\n");*/
		return;
	}

	delete[] buffer;
	printf("Shutdown");
	return;
}

void Client::sendMessage(char* data, int32_t length)
{
	int result;
	int size = 0;
	bool commandUsed;
	if (ClientCommandCheck(data))
	{
		
	}
	//length against 255 0
	if (length > 255 || length < 0)
	{
		printf("Parameter Error");
		return;
	}
	//Used to get the size of the message
	result = tcp_send_whole(ComSocket, (char*)&length, 1);
	if ((result == SOCKET_ERROR) || (result == 0))
	{
		int result = WSAGetLastError();
		printf("Disconnect");
		return;

	}
	//Send the actual message
	result = tcp_send_whole(ComSocket, data, length);
	if ((result == SOCKET_ERROR) || (result == 0))
	{
		int result = WSAGetLastError();
		printf("Disconnect");
		return;

	}
	else if (result == WSAESHUTDOWN || result == WSAEDISCON)
	{
		int result = WSAGetLastError();
		printf("Shutdown");
		return;
	}
	else
	{
		/*printf("%s: ", userUsername)*/;
		return;
	}
}

