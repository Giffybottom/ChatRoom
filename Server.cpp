#include "Server.h"

// Initialize both the master and ready set

Server::Server()
{
}

Server::~Server()
{
}

void Server::initServer()
{
	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	UDPSender = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	int errorCheck;

	if (listenSocket == INVALID_SOCKET)
	{
		printf("[Listen Socket Creation: Failure]\n");
		return;
	}
	else
	{
		printf("[Listen Socket Creation: Success]\n");
	}

	if (UDPSender == INVALID_SOCKET)
	{
		printf("[UDP Socket Creation: Failure]\n");
		return;
	}
	else
	{
		printf("[UDP Socket Creation: Success]\n");
	}

	errorCheck = setsockopt(UDPSender, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

	errorCheck = setsockopt(UDPSender, SOL_SOCKET, SO_REUSEADDR, &broadcast, sizeof(broadcast));

	//Bind
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);

	UDPAddress.sin_family = AF_INET;
	UDPAddress.sin_addr.S_un.S_addr = INADDR_BROADCAST;
	UDPAddress.sin_port = htons(31330);

	int result = bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (result == SOCKET_ERROR)
	{
		printf("[Bind Setup: Failure]\n");
		return;
	}
	else
	{
		printf("[Bind Setup: Success]\n");
	}

	//Listen
	result = listen(listenSocket, 1);
	if (result == SOCKET_ERROR)
	{
		printf("[Listen Setup: Failure]\n");
		return;
	}
	else
	{
		printf("[Listen Setup: Success]\n");
	}

	printf("[Waiting For Connect...]\n\n");


	FD_ZERO(&masterSet);
	FD_SET(listenSocket, &masterSet);
	FD_ZERO(&readySet);
	//FD_ZERO(&deniedSet);

	return;
}

void Server::updateServer()
{
	while (true)
	{
		timeval timeout;
		timeout.tv_sec = 1;

		int errorCheck;

		//App means append
		toServerLog_OStream.open("ServerLog.txt", std::ios_base::app);
		if (!toServerLog_OStream.is_open())
		{
			return;
		}

		readySet = masterSet;

		//return unless a socket is ready
		int readyFD = select(1, &readySet, NULL, NULL, &timeout);

		errorCheck = sendto(UDPSender, FullConnectionInfo, strlen(FullConnectionInfo) + 1,0,(SOCKADDR*)&UDPAddress, sizeof(UDPAddress));

		if (errorCheck == SOCKET_ERROR)
		{
			printf("UDP Send To Function Error\n");
		}

		for (size_t i = 0; i < readySet.fd_count; i++)
		{
			SOCKET s = readySet.fd_array[i];

			if (s == listenSocket)
			{

				if (CURR_USER_COUNT >= SV_FULL)
				{
					//Send Connected User SV_FULL Message
					SOCKET F = accept(listenSocket, NULL, NULL);
					sendMessageSingleUser(F, serverFullMessage, sizeof(serverFullMessage));
					return;
				}
				else
				{
					SOCKET sock = accept(listenSocket, NULL, NULL);
					FD_SET(sock, &masterSet);
					CURR_USER_COUNT++;
					sendMessageSingleUser(sock, serverConnectMessage, sizeof(serverConnectMessage));
					return;
				}
			}
			else
			{
				char recievedMessage[256] = { '\0' };
				readMessage(s, recievedMessage, 256);

				//if message is a command do this
				if (recievedMessage[0] == '$')
				{
					printf("[Command Entered: %s]\n", recievedMessage);

					//Parse command for validation
					std::string s_recievedMessage = recievedMessage;
					std::string command;

					toServerLog_OStream << s_recievedMessage << '\n';

					//Identify Command
					//detect first bracket
					std::size_t leftAngleBracket = s_recievedMessage.find_first_of('<');
					std::size_t rightAngleBracket;

					if (leftAngleBracket != std::string::npos)
					{
						rightAngleBracket = s_recievedMessage.find_last_of('>');

						//Example: $Register<Giffybottom>
						command = s_recievedMessage.substr(1, leftAngleBracket - 1); // Register

						username = s_recievedMessage.substr(leftAngleBracket + 1, rightAngleBracket - leftAngleBracket - 1);
					}
					else
					{
						command = s_recievedMessage.substr(1, s_recievedMessage.size());
					}

					COMMANDS currCommand = COMMANDS::UNKNOWN;

					if (command == "Register" || command == "register")
					{
						currCommand = COMMANDS::Register;
					}
					else if (command == "Exit" || command == "exit")
					{
						currCommand = COMMANDS::Exit;
					}
					else if (command == "Getlist" || command == "GetList" || command == "getlist" || command == "getList")
					{
						currCommand = COMMANDS::Getlist;
					}
					else if (command == "Getlog" || command == "GetLog" || command == "getlog" || command == "getLog")
					{
						currCommand = COMMANDS::Getlog;
					}
					else
					{

					}

					//Message to user created.
					std::string blankMessage = " ";
					std::string registerUserMessage = " has been registered.\n";
					blankMessage.append(username);
					blankMessage.append(registerUserMessage);

					std::string temp = " left!\n";

					//Do Command
					switch (currCommand)
					{
					case Register:
						//Do register stuff
						//Server Messages String
						
						//if username is already s already has a username reassign username
						if (registeredUsers.find(s) == registeredUsers.end())
						{
							registeredUsers.insert(std::pair<SOCKET, std::string>(s, username));
							sendMessageAllUsers(blankMessage);
							printf("User Registered (Message Sent)\n");
							toServerLog_OStream << blankMessage << '\n';
						}
						else
						{
							registeredUsers[s] = username;
						}

						break;
					case Exit:
						//Do exit stuff						
						toServerLog_OStream << registeredUsers[s] << temp << '\n';
						blankMessage.append(" ");
						blankMessage.append(registeredUsers[s]);
						blankMessage.append(temp);

						for (size_t x = 0; x < masterSet.fd_count; x++)
						{
							SOCKET sendSocket = masterSet.fd_array[x];
							if (sendSocket != listenSocket)
								sendMessageSingleUserString(sendSocket, blankMessage);
						}

						registeredUsers.erase(s);
						FD_CLR(s, &masterSet);


						break;
					case Getlist:
						//Get a list??
						
						sendMessageSingleUserString(s, " Registered Users: ");
						toServerLog_OStream << " Registered Users: ";

						for (auto i = registeredUsers.begin(); i != registeredUsers.end(); i++)
						{
							blankMessage = "";
							blankMessage.append(i->second);
							blankMessage.append(", ");
							sendMessageSingleUserString(s, blankMessage);
							toServerLog_OStream << blankMessage;
						}

						sendMessageSingleUserString(s, "\n");
						toServerLog_OStream << "\n";

						break;
					case Getlog:
						//Get a log
						toServerLog_OStream.close();
						fromServerLog_IStream.open("ServerLog.txt");

						if (!fromServerLog_IStream.is_open())
						{
							printf("File did not open\n");
							break;
						}

						std::string logMessage = "";
						while (std::getline(fromServerLog_IStream, blankMessage))
						{
							logMessage.append(blankMessage);
							logMessage.append("\n");
							sendMessageSingleUserString(s, logMessage);
							logMessage = "";
						}
						logMessage = "SV_END_FILE";
						sendMessageSingleUserString(s, logMessage);

						break;
					}

					//printf("DEBUG LIES!");
				}
				else
				{
					std::string output = recievedMessage;
					output.append("\n");

					std::string fullMessage = " ";
					fullMessage.append(registeredUsers[s]);
					fullMessage.append(": ");
					fullMessage.append(output);

					//resend message to all sockets
					char* finalOutput = new char[fullMessage.length() + 1];
					strncpy(finalOutput, fullMessage.c_str(), (fullMessage.length() + 1));

					//Log Message
					toServerLog_OStream << fullMessage << '\n';

					//Print to Server Console Window
					printf(finalOutput);

					int counter = 0;

					//for loop mainset				
					for (size_t j = 0; j < masterSet.fd_count; j++)
					{
						SOCKET m = masterSet.fd_array[j];

						//make sure not listen socket
						if (m != listenSocket)
						{
							//check main socket vs current before sending
							if (s != m)
							{
								sendMessageSingleUser(m, finalOutput, (fullMessage.length() + 1));
							}
						}
					}
					delete[] finalOutput;
				}
			}
		}

		//save your s***
		toServerLog_OStream.close();
	}
}

bool Server::ServerFullCheck()
{
	if (CURR_USER_COUNT == SV_FULL)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//std::string Server::GetListOfRegisteredUsers()
//{
//	std::string output = " Registered Users: ";
//	for (auto i = registeredUsers.begin(); i != registeredUsers.end(); i++)
//	{
//		output.append(i->second);
//		output.append(", ");
//	}
//	output.append("\n");
//	return output;
//}

void Server::sendMessageAllUsers(std::string data)
{
	//for loop mainset				
	for (size_t j = 0; j < masterSet.fd_count; j++)
	{
		SOCKET m = masterSet.fd_array[j];

		//make sure not listen socket
		if (m != listenSocket)
		{
			//check main socket vs current before sending
			std::string output = data;
			//resend message to all sockets
			char* finalOutput = new char[output.length() + 1];
			strncpy(finalOutput, output.c_str(), (output.length() + 1));
			sendMessageSingleUser(m, finalOutput, (output.length() + 1));

			delete[] finalOutput;
		}

	}
}

void Server::sendMessageSingleUserString(SOCKET s, std::string message)
{
	std::string output = message;
	//resend message to all sockets
	char* finalOutput = new char[output.length() + 1];
	strncpy(finalOutput, output.c_str(), (output.length() + 1));
	sendMessageSingleUser(s, finalOutput, (output.length() + 1));

	delete[] finalOutput;
}

void Server::sendMessageSingleUser(SOCKET s, char* data, int32_t length)
{
	int result;
	int size = 0;

	//length against 256 0
	if (length > 256 || length < 0)
	{
		printf("Parameter Error singleUser\n");
		return;
	}
	//Used to get the size of the message
	result = tcp_send_whole(s, (char*)&length, 1);
	if ((result == SOCKET_ERROR) || (result == 0))
	{
		int result = WSAGetLastError();
		printf("Disconnect\n");
		return;

	}
	//Send the actual message
	result = tcp_send_whole(s, data, length);
	if ((result == SOCKET_ERROR) || (result == 0))
	{
		int result = WSAGetLastError();
		printf("Disconnect\n");
		return;

	}
	else if (result == WSAESHUTDOWN || result == WSAEDISCON)
	{
		int result = WSAGetLastError();
		printf("Shutdown\n");
		return;
	}
	else
	{
		if (data[0] == '0' && data[1] == '0')
		{
			printf("[Users Connect Attempt Failure: Server Full]\n");
		}
		else if (data[0] == '1' && data[1] == '1')
		{
			printf("[Users Connect Attempt Success: User Count (%d/%d)]\n", CURR_USER_COUNT, SV_FULL);
		}
		return;
	}
}

void Server::closeServer()
{
	shutdown(listenSocket, SD_BOTH);
	closesocket(listenSocket);

	/*shutdown(ComSocket, SD_BOTH);
	closesocket(ComSocket);*/
}

void Server::readMessage(SOCKET s, char* buffer, int32_t size)
{
	int result;
	int counterOfBytes = 0;
	if (sizeof(buffer) > size)
	{
		printf("Parameter Error");
		return;
	}
	result = tcp_recv_whole(s, (char*)&counterOfBytes, 1);
	if ((result == SOCKET_ERROR) || (result == 0))
	{
		result = WSAGetLastError();
		printf("Disconnect");
		return;

	}

	result = tcp_recv_whole(s, (char*)buffer, counterOfBytes);
	if ((result == SOCKET_ERROR) || (result == 0))
	{
		result = WSAGetLastError();
		printf("Disconnect");
		return;
	}
	else
	{
		printf("Message Recieved\n");
		return;
	}

	delete[] buffer;
	printf("Shutdown");
	return;
}


