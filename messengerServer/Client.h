#pragma once
#include <iostream>
#include <windows.h>
class Client
{
public:
	Client(void);
	~Client(void);

	SOCKET _clientSocket;
	int _numberOfClient;
	bool _isDisconnected;
};

