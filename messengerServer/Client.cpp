#include "StdAfx.h"
#include "Client.h"


Client::Client(void):
	_clientSocket(INVALID_SOCKET),
	_isDisconnected(false)
{

}


Client::~Client(void)
{
}
