// Brandon Zaks
// CSCE 5580 Computer Networks
// File: main.cpp

#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string>

#define SERVER_IP "129.120.151.97" //cse04.cse.unt.edu
#define SERVER_PORT	60000
#define LISTEN_PORT	60000

using namespace std;
//class used for TCP connections
class TCP_Connection
{
	public:
		char buffer[5000000];
		ssize_t bytesReceived;

		void CreateInboundSocket();//create an available inbound connection to this program
		void ReceiveData();//wait in a loop for incomming data
		void CreateOutboundSocket(string ip, unsigned short port);//create an outbound connection to some server
		void SendToOutboundSocket(char* buffer, int size);//send data to server
    ~TCP_Connection();//close connection

	private:
		int socketListenDescriptor, socketAcceptDescriptor, socketSendDescriptor;
		socklen_t remoteEndpointAddressSize, remoteInboundEndpointAddressSize;
		struct sockaddr_in localEndpointAddress, remoteEndpointAddress, remoteInboundEndpointAddress;
};