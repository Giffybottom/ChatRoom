// NetworkingProject.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "Client.h"
#include "Server.h"

int main()
{
	Server server;
	Client client;

	startup();
    
	do
	{
		int choice;
		do
		{
			printf("Would you like to Create a Server or join as a Client?\n");
			printf("1> Client\n");
			printf("2> Server\n");
			printf("3> Exit\n");
			std::cin >> choice;
		} while (choice != 1 && choice != 2 && choice != 3);

		//Client
		if (choice == 1)
		{
			std::cin.clear();
			if (client.initClient())
			{
				do
				{

				} while (client.updateClient());
				choice = 3;
			}
			
		}


		//Server
		if (choice == 2)
		{
			server.initServer();

			do
			{
				server.updateServer();

			} while (true);
		}

		if (choice == 3)
			break;

	} while (true);

	shutdown();
}


