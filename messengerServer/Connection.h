#pragma once

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512

#include <iostream>
#include <sstream>
#include <vector>
#include "Client.h"
#include <algorithm> 

class Connection
{
public:
	Connection(void);
	~Connection(void);

	void SetConnectionConfig(std::string serv_port);
	void initializeWinsock();
	void resolveServerAddress();
	void createSocket();
	void setupTCP();
	void listenForNewClient();
	void acceptNewClient();
	void startConnect(int clientID);

private:
	WSADATA _wsaData;
	SOCKET _listenSocket;
	SOCKET _acceptedSocket;

	int _iResult;
    int _iSendResult;
	char* _port;

	HANDLE _hArray[20];

	Client client[100];

};

