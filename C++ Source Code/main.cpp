#include "TCP_Connection.h"
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <iostream>

using namespace std;

#define PORT 80
#define USERAGENT "Proxy Server"

char *build_get_query(string host, string page)
{
  char *query;
  //char *getpage = page.c_str();
  char *tpl = "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-agent: %s\r\n\r\n";
  
  if(page.c_str()[0] == '/')
  {
    //getpage = getpage + 1;
    fprintf(stderr,"Removing leading \"/\", converting %s to %s\n", page.c_str(), page.c_str());
  }
  // -5 is to consider the %s %s %s in tpl and the ending \0
  query = (char *)malloc(strlen(host.c_str())+strlen(page.c_str())+strlen(USERAGENT)+strlen(tpl)-5);
  sprintf(query, tpl, page.c_str(), host.c_str(), USERAGENT);
  
  return query;
}

char *get_ip(string host)
{
  struct hostent *hent;
  int iplen = 15; //XXX.XXX.XXX.XXX
  char *ip = (char *)malloc(iplen+1);
  memset(ip, 0, iplen+1);

  if((hent = gethostbyname(host.c_str())) == NULL)
  {
    herror("Can't get IP");
    exit(1);
  }

  if(inet_ntop(AF_INET, (void *)hent->h_addr_list[0], ip, iplen) == NULL)
  {
    perror("Can't resolve host");
    exit(1);
  }

  return ip;
}

int main()
{
  char *get;
  char *host;
  unsigned short port = 80;
  //char *page;

  //char* ipAddress;
  string URL, ipAddress, page;
  TCP_Connection tcp_Connection;
	
  //Create connection for inbound connections
	tcp_Connection.CreateInboundSocket();
 //wait for data to come in
  tcp_Connection.WaitForInboundConnection();
//copy header into header struct
	fprintf(stderr, "Query is:\n<<START>>\n%s<<END>>\n", tcp_Connection.buffer);
 
 int startPosition, endPosition;
 startPosition = endPosition = 0;
   for (int i = 0; i < 5000000; i++)
   {
     if (tcp_Connection.buffer[i] == 47 && startPosition == 0)
     {
       startPosition = i + 1;
     }
     
     if (tcp_Connection.buffer[i] == 32 && startPosition > 0)
     {
       endPosition = i;
       break;
     }
   }
   
   URL.assign(&tcp_Connection.buffer[startPosition],endPosition - startPosition);
   
   cout << URL << endl;
  
  
  
  page = "/";
  
  ipAddress.assign(get_ip(URL));
  
  cout << ipAddress;
  
  get = build_get_query(URL, page);
  fprintf(stderr, "Query is:\n<<START>>\n%s<<END>>\n", get);
  
  tcp_Connection.CreateOutboundSocket(ipAddress, port);
  
  tcp_Connection.SendToOutboundSocket(get, strlen(get));
  
  tcp_Connection.ReceiveData();
  
  fprintf(stderr, "Query is:\n<<START>>\n%s<<END>>\n", tcp_Connection.buffer);
  
  tcp_Connection.SendData(tcp_Connection.buffer, strlen(tcp_Connection.buffer));
  
  tcp_Connection.CreateInboundSocket();
  tcp_Connection.WaitForInboundConnection();
//copy header into header struct
	fprintf(stderr, "Query is:\n<<START>>\n%s<<END>>\n", tcp_Connection.buffer);
  
  while (true)
  {
  
 
 
 startPosition = endPosition = 0;
   for (int i = 0; i < 5000000; i++)
   {
     if (tcp_Connection.buffer[i] == 47 && startPosition == 0)
     {
       startPosition = i + 1;
     }
     
     if (tcp_Connection.buffer[i] == 32 && startPosition > 0)
     {
       endPosition = i;
       break;
     }
   }
   
   page.assign(&tcp_Connection.buffer[startPosition],endPosition - startPosition);
   
   cout << page << endl;
   
   //page = "/";
   
   get = build_get_query(URL, page);
  fprintf(stderr, "Query is:\n<<START>>\n%s<<END>>\n", get);
 
 
  
  tcp_Connection.CreateOutboundSocket(ipAddress, port);
  
  tcp_Connection.SendToOutboundSocket(get, strlen(get));
  
  memset(tcp_Connection.buffer, 0, 5000000);
  tcp_Connection.ReceiveData();
  
  fprintf(stderr, "Query is:\n<<START>>\n%s<<END>>\n", tcp_Connection.buffer);
  
  tcp_Connection.SendData(tcp_Connection.buffer, strlen(tcp_Connection.buffer));
  
  tcp_Connection.CreateInboundSocket();
  tcp_Connection.WaitForInboundConnection();
//copy header into header struct
	fprintf(stderr, "Query is:\n<<START>>\n%s<<END>>\n", tcp_Connection.buffer);
  }
 
  

  
  //printf("%s", ipAddress);
  //TCP_Connection tcp_Connection;
  
  return 1;
}