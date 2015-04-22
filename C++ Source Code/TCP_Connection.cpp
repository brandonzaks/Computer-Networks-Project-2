// Brandon Zaks
// CSCE 5580 Computer Networks
// File: main.cpp

#include "TCP_Connection.h"

void TCP_Connection::CreateInboundSocket()
{
  close(socketAcceptDescriptor);//close socketAcceptDescriptor
  close(socketListenDescriptor);//close socketListenDescriptor
  
	int optionValue = 1;
	int result = 0;
  //zero out localEndpointAddress struct
	memset(&localEndpointAddress, 0, sizeof(localEndpointAddress));
	localEndpointAddress.sin_family = AF_INET;//we are using AF_INET family connection
	localEndpointAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	localEndpointAddress.sin_port = htons((unsigned short)LISTEN_PORT);//port we will listen on defined in .h file

	socketListenDescriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//create new socket meant for listening
	//appropriate error message if fail
	if (socketListenDescriptor == -1)
	{
		printf("socket() Failed\n");
    printf("Error: %s\n",strerror(errno));
		exit(0);
	}
	
	else
		printf("socket() Success\n");
	//set this socket option to allow connections to the same IP.
	result = setsockopt(socketListenDescriptor, SOL_SOCKET, SO_REUSEADDR, (const char *)&optionValue, sizeof(optionValue));
  //appropriate error message if fail
	if (result == -1)
	{
		printf("setsockopt() Failed\n");
    printf("Error: %s\n",strerror(errno));
		close(socketListenDescriptor);
		exit(0);
	}

	else
		printf("setsockopt() Success\n");
	//bind listen descriptor to local endpoint address
	result = bind(socketListenDescriptor, (struct sockaddr *)&localEndpointAddress, sizeof(localEndpointAddress));
  //appropriate error message if fail
	if (result == -1)
	{
		printf("bind() Failed\n");
    printf("Error: %s\n",strerror(errno));
		close(socketListenDescriptor);
		exit(0);
	}

	else
		printf("bind() Success\n");
  //listen for connections
	result = listen(socketListenDescriptor, 5);
  //appropriate error message if fail
	if (result == -1)
	{
		printf("listen() Failed\n");
    printf("Error: %s\n",strerror(errno));
		close(socketListenDescriptor);
		exit(0);
	}

	else
		printf("listen() Success\n");

	remoteInboundEndpointAddressSize = sizeof(sockaddr_in);
	socketAcceptDescriptor = accept(socketListenDescriptor, (struct sockaddr *)&remoteInboundEndpointAddress, &remoteInboundEndpointAddressSize);//accept connections to socket
  //appropriate error message if fail
	if (socketAcceptDescriptor == -1)
	{
		printf("accept() Failed\n");
    printf("Error: %s\n",strerror(errno));
		close(socketListenDescriptor);
		exit(0);
	}

	else
		printf("accept() Success\n");
}

void TCP_Connection::WaitForInboundConnection()
{
  memset(buffer, 0, 5000000);
    
  bytesReceived = 0;
    
	while (true)
	{
		bytesReceived = recv(socketAcceptDescriptor, buffer, sizeof(buffer), 0);

		if (bytesReceived == -1)
		{
			printf("recv() Failed\n");
      printf("Error: %s\n",strerror(errno));
			exit(0);
		}

		else if (bytesReceived > 0)
			printf("recv() Success\n");

		return;
	}
}

void TCP_Connection::SendData(char* buffer, int size)
{
	ssize_t bytesSent;

  int sent = 0;
  
  while(sent < size)
  {
    bytesSent = send(socketAcceptDescriptor, buffer+sent, size-sent, 0);//send byte array to server
    
    //handle error
    if (bytesSent == -1)
    {
      printf("send() Failed\n");
      printf("Error: %s\n",strerror(errno));
      exit(0);
    }
    
    sent += bytesSent;
  }
	//else
		//printf("send() Success\n");
}

void TCP_Connection::ReceiveData()
{
  memset(buffer, 0, 5000000);
    
  bytesReceived = 0;
  //continue to check for new data coming in until we break out
	while ((bytesReceived = recv(socketSendDescriptor, &buffer[bytesReceived], sizeof(buffer), 0)) > 0)
	{
		//bytesReceived = recv(socketSendDescriptor, buffer, sizeof(buffer), 0);//receieve data into a byte buffer
    //error message if we get -1
		if (bytesReceived == -1)
		{
			printf("recv() Failed\n");
      printf("Error: %s\n",strerror(errno));
			exit(0);
		}

		else if (bytesReceived > 0)
			printf("recv() Success\n");
	}
}

void TCP_Connection::CreateOutboundSocket(string ip, unsigned short port)
{
	int result;

	memset(&remoteEndpointAddress, 0, sizeof(remoteEndpointAddress));//zero remoteEndpointAddress struct
	remoteEndpointAddress.sin_family = AF_INET;//set AF_INET for family
	remoteEndpointAddress.sin_addr.s_addr = inet_addr(ip.c_str());//set serverIP from .h file
	remoteEndpointAddress.sin_port = htons(port);//set serverPort from .h file

	socketSendDescriptor = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);//create new socket meant for sending data
  //error message if something goes wrong
	if (socketSendDescriptor == -1)
	{
		printf("socket() Failed\n");
    printf("Error: %s\n",strerror(errno));
		exit(0);
	}

	else
		printf("socket() Success\n");
	//connect to remote address to establish connection
	result = connect(socketSendDescriptor, (struct sockaddr *) &remoteEndpointAddress, sizeof(remoteEndpointAddress));
  //error message if something goes wrong
	if (result == -1)
	{
		printf("connect() Failed\n");
    printf("Error: %s\n",strerror(errno));
		close(socketSendDescriptor);
		exit(0);
	}

	else
		printf("connect() Success\n");
}

void TCP_Connection::SendToOutboundSocket(char* buffer, int size)
{
	ssize_t bytesSent;

  int sent = 0;
  
  while(sent < size)
  {
    bytesSent = send(socketSendDescriptor, buffer+sent, size-sent, 0);//send byte array to server
    
    //handle error
    if (bytesSent == -1)
    {
      printf("send() Failed\n");
      printf("Error: %s\n",strerror(errno));
      exit(0);
    }
    
    sent += bytesSent;
  }
	//else
		//printf("send() Success\n");
}

TCP_Connection::~TCP_Connection()
{
  close(socketAcceptDescriptor);//close socketAcceptDescriptor
  close(socketSendDescriptor);//close socketSendDescriptor
  close(socketListenDescriptor);//close socketListenDescriptor
}