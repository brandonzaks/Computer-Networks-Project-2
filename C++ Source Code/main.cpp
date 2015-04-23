#include "TCP_Connection.h"
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "Database.h"

Database database;

using namespace std;

#define PORT 80
#define USERAGENT "Proxy Server"

void cleanLanguage(char* buffer, int length)
{
  string vulgarWord = "";
  int matchCount = 0;
  
  for (int i = 0; i < length; i++)
  {
    for (int j = 1; j < 2; j++)
    {
      vulgarWord = database.get_vulgar_word(j);
      
      if (vulgarWord.size() <= length - i)
      {
        matchCount = 0;
        
        for (int k = 0; k < vulgarWord.size(); k++)
        {
          if (vulgarWord.c_str()[k] == buffer[i + k])
          {
            matchCount++;
          }
          
          else
          {
            break;
          }
        }
        
        if (matchCount == vulgarWord.size())
        {
          for (int m = 0; m < vulgarWord.size(); m++)
          {
            buffer[i + m] = '!';
          }
        }
      }
    }
  } 
}

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

char *build_400()
{
  char tpl[] = "HTTP/1.0 400 Bad Request\r\nConnection: close\r\n\r\n";
  
  return tpl;
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
  char error[] = "HTTP/1.0 400 Bad Request\r\nConnection: close\r\n\r\n";
  char success[] = "HTTP/1.0 200 OK\r\nConnection: close\r\n\r\n";
  char *host;
  unsigned short port = 80;
  char* buffer;
  string newurl = "";
  string URL, ipAddress, page;
  TCP_Connection tcp_Connection;
  int startPosition, endPosition;
 startPosition = endPosition = 0;
  
	tcp_Connection.CreateInboundSocket();
  
  tcp_Connection.WaitForInboundConnection();
  
  fprintf(stderr, "Query is:\n<<Browser Request>>\n%s<<Browser Request>>\n", tcp_Connection.buffer);
 
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
   
   URL.assign(&tcp_Connection.buffer[startPosition], endPosition - startPosition);
   
   cout << "URL Passed from browser: " << URL << endl;
   
   int count2 = 0;
   for (int i = 8; i < URL.size(); i++)
   {
     if (URL.c_str()[i] == '/')
     {
       count2 = i;
       break;
     }
   }
  
  newurl = URL.substr(7, count2 - 7);
  
  cout << "New URL created from above URL: " << newurl << endl;
  
  page = URL.substr(count2, URL.size() - count2);
  
  if (newurl.size() == (URL.size() - 7))
    page = "/";
  
  cout << "Requested Page: " << page << endl;
  
  ipAddress.assign(get_ip(newurl));
  
  cout << "IP Address of requested site: " << ipAddress << endl;
  
  if (database.is_blacklisted(newurl) == true)
  {
    tcp_Connection.SendData(error, strlen(error));
    fprintf(stderr, "Query is:\n<<Error Response to Browser>>\n%s<<Error Response to Browser>>\n", error);
  }
  
  else
  {
    get = build_get_query(newurl, page);
    fprintf(stderr, "Query is:\n<<Request Message to URL>>\n%s<Request Message to URL>>\n", get);
    
    tcp_Connection.CreateOutboundSocket(ipAddress, port);
    
    tcp_Connection.SendToOutboundSocket(get, strlen(get));
    
    tcp_Connection.ReceiveData();
    
    int location = 0;
    for (int i = 0; i < 5000000; i++)
    {
      if (i < (5000000 - 4) && tcp_Connection.buffer[i] == 13 && tcp_Connection.buffer[i+1] == 10 && tcp_Connection.buffer[i+2] == 13 && tcp_Connection.buffer[i+3] == 10)
      {
        location = i;
        break;
     }
   }
   
   buffer = new char[5000000 - location+4];
   
   int count = 0;
   for (int i = location+4; i < strlen(tcp_Connection.buffer); i++)
   {
     buffer[count] = tcp_Connection.buffer[i];
     count++;
   }
   
   struct stat st = {0};
   
   string FilePath = database.get_website_file_path(URL);
   string Location = "";
   string Location2 = "";
   
   if (FilePath == "")
   {
     cleanLanguage(buffer, count - 1);
     
     int count3 = 0;
     for (int i = 0; i < strlen(success); i++)
     {
       tcp_Connection.buffer[i] = success[i];
       count3++;
     }
     
     database.add_website_file_path(URL, "index.html", "FileStore/" + newurl + "/");
     Location = "FileStore/" + newurl + "/";
     Location2 = "FileStore/" + newurl;
     cout << Location << endl;
     cout << Location2 << endl;
     if (stat(Location2.c_str(), &st) == -1)
     {
       mkdir(Location2.c_str(), 0700);
     }
     
     FilePath = Location + "index.html";
     cout << FilePath << endl;
     std::ofstream file(FilePath.c_str(), std::ios::binary);
     
     file.write(&buffer[0], strlen(buffer));
     
     for (int i = 0; i < strlen(buffer); i++)
     {
       tcp_Connection.buffer[i+count3] = buffer[i];
     }
     
     delete[] buffer;
   }
   
   else
   {
     int count3 = 0;
     for (int i = 0; i < strlen(success); i++)
     {
       tcp_Connection.buffer[i] = success[i];
       count3++;
     }
   
     std::ifstream is(FilePath.c_str(), std::ifstream::binary);
     
     is.seekg(0, is.end);
     int length = is.tellg();
     is.seekg(0, is.beg);

     char* buffer5 = new char[length];

     is.read(buffer5,length);
     
     for (int i = 0; i < strlen(buffer5); i++)
     {
       tcp_Connection.buffer[i+count3] = buffer5[i];
     }
     
     delete[] buffer5;
   }
   
  fprintf(stderr, "\nQuery is:\n<<START>>\n%s<<END>>\n\n", tcp_Connection.buffer);
  
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
 
  
}
  
  //printf("%s", ipAddress);
  //TCP_Connection tcp_Connection;
  
  return 1;
}
