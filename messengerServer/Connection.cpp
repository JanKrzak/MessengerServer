#include "StdAfx.h"
#include "Connection.h"

using namespace std;

DWORD WINAPI ClientThreadFunction(LPVOID client_param);

//Addres storage structure has information about IP addres
struct addrinfo *result = NULL;

//Data storage structure has information about addres
struct addrinfo hints;

//vector of Client class objects
std::vector<Client> clientVector;

Connection::Connection(void)
{

}

Connection::~Connection(void)
{
}

/*
// \brief  Function set connection parameters: port number
*/
void Connection::SetConnectionConfig(std::string serv_port)
{
	stringstream sPort;
	sPort << serv_port;
	_port  = new char [10];
	sPort >> _port;
}

/*
// \brief  Function initialize Winsock
*/
void Connection::initializeWinsock()
{
	_iResult = WSAStartup(MAKEWORD(2,2), &_wsaData);
	if (_iResult != 0) 
	{
		cout << "WSAStartup failed with error: " << _iResult <<endl;
		return;
	}
}

/*
// \brief  Function resolve server addres
*/
void Connection::resolveServerAddress()
{
	_iResult = getaddrinfo(NULL, _port, &hints, &result);
	if ( _iResult != 0 ) 
	{
		cout << "getaddrinfo failed with error: " << _iResult <<endl;
		WSACleanup();
		return;
	}
}

/*
// \brief  Function create SOCKET
*/
void Connection::createSocket()
{
	_listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (_listenSocket == INVALID_SOCKET)
	{
		cout << "socket failed with error: " << WSAGetLastError() <<endl;
		freeaddrinfo(result);
		WSACleanup();
		return;
	}
}

/*
// \brief  Function setup TCP
*/
void Connection::setupTCP()
{
	_iResult = bind( _listenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (_iResult == SOCKET_ERROR)
	{
		cout << "bind failed with error: " << WSAGetLastError() <<endl;
		freeaddrinfo(result);
		closesocket(_listenSocket);
		WSACleanup();
		return;
	}
}

/*
// \brief Function listen for new clients
*/
void Connection::listenForNewClient()
{
	_listenSocket = INVALID_SOCKET;
	_acceptedSocket = INVALID_SOCKET;

	_iResult = NULL;
	_iSendResult = NULL;

	initializeWinsock();

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	resolveServerAddress();

	createSocket();

	setupTCP();

	freeaddrinfo(result);

	_iResult = listen(_listenSocket, SOMAXCONN);

	if (_iResult == SOCKET_ERROR) 
	{
		cout << "listen failed with error: " << WSAGetLastError() <<endl;
		closesocket(_listenSocket);
		WSACleanup();
		return;
	}

	// Accept a client socket
	_acceptedSocket = accept(_listenSocket, NULL, NULL);		//function stop here and listen until the new client is connected

	if (_acceptedSocket == INVALID_SOCKET) 
	{
		cout << "accept failed with error: " << WSAGetLastError() <<endl;
		closesocket(_listenSocket);
		WSACleanup();
		return;
	}

	closesocket(_listenSocket);

	acceptNewClient();
}


/*
// \brief Function create new client object and assigns client SOCKET and current client number
*/
void Connection::acceptNewClient()
{
	static int clientNumber = 0;

	client[clientNumber]._clientSocket = _acceptedSocket;

	clientVector.push_back(client[clientNumber]);		//push new client object to vector

	client[clientNumber]._numberOfClient = clientNumber;

	//create new thread for specific client to listen for new message and to send them to others clients
	_hArray[clientNumber] = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)ClientThreadFunction, &client[clientNumber], 0, NULL);	

	clientNumber++;
}

/*
// \brief Function initialize client name
*/
int initUsername(Client& client, char userName[512], char* wordToSend)
{
	int iResultName = 0;
	int recvbuflen = 512;
	iResultName = recv(client._clientSocket, userName, recvbuflen, 0);
	if ( iResultName > 0 )
	{
		cout<<"\rRecived Name: ";
		for(int i = 0; i < iResultName; i++)
		{
			cout << userName[i];
			wordToSend[i] = userName[i];
			wordToSend[i + 1] = ':';
			wordToSend[i + 2] = ' ';
		}
		cout<<"\n";
	}
	return iResultName + 2;
}

/*
// \brief Function send message to uthers clients connected to sesrver
// \param wordToSend[512] = provide message with client name at begin,
// \param iResult - provide lenght of message
// \param nameLenght - provide lenght of client name
// \param threadClientVector - has information about the current object
// \param currentClient - provide current client number
*/
void sendMessage(Client& client, char wordToSend[512], int iResult, int nameLenght, vector <Client> threadClientVector, int currentClient)
{
			
			int iSendResult = 0;
			threadClientVector = clientVector;			
			threadClientVector.erase(threadClientVector.begin() + currentClient);		//erase current client form list. Everyone else except you should receive a message. 

			for(unsigned i = 0; i < threadClientVector.size(); i++)
			{
				if(!threadClientVector[i]._isDisconnected)
				{
					iSendResult = send( threadClientVector[i]._clientSocket, wordToSend, iResult + nameLenght, 0 );		//send message to everyone
				}

				// Echo the buffer back to the sender
				if (iSendResult == SOCKET_ERROR) 
				{
					cout << "send failed with error: " << WSAGetLastError() <<endl;
					closesocket(client._clientSocket);
					WSACleanup();
					return;
				}
			}
			cout << "Bytes sent: " << iSendResult <<endl;
}

/*
// \brief New thread Function to listen and send message to others clients
// \param LPVOID client_param - provide current client data
*/
DWORD WINAPI ClientThreadFunction(LPVOID client_param)
{
	char recvbuf[512];
	char userName[512];
	char wordToSend[512];
	int recvbuflen = 512;
	int iResult = 0;
	int nameLenght = 0;
	bool isExit = false;
	vector <Client> threadClientVector;

	Client* client;

	client = static_cast<Client*>(client_param);
	int currentClient = client->_numberOfClient;

	nameLenght = initUsername(*client, userName, wordToSend);		//init username and return lenght of current word
	
	cout << "Connected to user: " << currentClient << endl;
	while(!isExit)
	{
		// Receive until the peer shuts down the connection
		iResult = recv(client->_clientSocket, recvbuf, recvbuflen, 0);
		std::cout << "Bytes received: " << iResult <<endl;

		if (iResult > 0) 
		{
			//writing recived message to character which contains client name at begin
			for( int i = 0; i < iResult; i++)
			{
				wordToSend[i + nameLenght] = recvbuf[i];
			}
			sendMessage(*client, wordToSend, iResult, nameLenght, threadClientVector, currentClient);
		}
		else if (iResult == 0)
		{
			isExit = true;
			cout << "Connection closing...\n" << endl;
		}
		else 
		{
			isExit = true;
			printf("recv failed with error: %d\n", WSAGetLastError());
		}
	}

	clientVector[currentClient]._isDisconnected = true;

	// shutdown the connection since we're done
	iResult = shutdown(client->_clientSocket, SD_SEND);

	if (iResult == SOCKET_ERROR)
	{
		cout << "shutdown failed with error: " << WSAGetLastError() << endl;
		closesocket(client->_clientSocket);
		WSACleanup();
		cout << "Disconnected from user: " << currentClient << endl;
		return 0;
	}
	closesocket(client->_clientSocket);
	WSACleanup();
	cout << "Disconnected from user: " << currentClient << endl;
	return 0;
}