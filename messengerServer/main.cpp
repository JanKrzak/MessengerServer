// messengerServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <sstream>
#include "Connection.h"

using namespace std;

#define DEFAULT_PORT "12345"

int main() 
{
	Connection connect;

	connect.SetConnectionConfig(DEFAULT_PORT);

	while(1)
	{
		connect.listenForNewClient();
	}

	return 0;
}

