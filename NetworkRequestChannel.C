
#include <cassert>
#include <pthread.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "NetworkRequestChannel.H"

using namespace std;

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/
    struct sockaddr_in server_in;
    struct sockaddr_storage serverStorage;
    int serverSocket, read_size;

    char buffer[1024], message[1024];
/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

    bool keep_going = true;

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* PRIVATE METHODS FOR CLASS   R e q u e s t C h a n n e l  */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR FOR CLASS   R e q u e s t C h a n n e l  */
/*--------------------------------------------------------------------------*/

//For the client
NetworkRequestChannel::NetworkRequestChannel(const string _host_name, const unsigned short _port_no) 
{
  cout<<"***HOST NAME: "<<_host_name<<'\n'<<endl;
  struct sockaddr_in serv_addr;
  struct hostent *hstnt;
  stringstream ss;
  ss << _port_no;
  string temp = ss.str();
  const char * port = temp.c_str();
  const char * _server_host_name = _host_name.c_str();

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  cout<<"AF_INET run, CLIENT SIDE"<<endl;
  serv_addr.sin_port = htons(_port_no);
  cout<<"sin_port run, CLIENT SIDE"<<endl;

  if(hstnt = gethostbyname(_host_name.c_str()))
  {
    memcpy(&serv_addr.sin_addr, hstnt->h_addr, hstnt->h_length);
  }
  else if(serv_addr.sin_addr.s_addr = inet_addr(_host_name.c_str()) == htonl(INADDR_NONE))
  {
    cout<<"FAILED: Cannot determine host: "<<_host_name.c_str()<<endl;
    cout<<"Hostent == "<<hstnt<<". Host by name == "<<gethostbyname(_host_name.c_str())<<". CLIENT SIDE"<<endl;
  }
  fd = socket(AF_INET, SOCK_STREAM, 0);
  if(fd < 0)
    cout<<"FAILED: Can't creat socket on client side."<<endl;

  if((connect(fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
  {
    cout<<"FAILED: Can't connect to " << _host_name.c_str() <<":"<<_port_no<<" on client side."<<endl;
    cout<<"Socket D == "<<fd<<endl;
    cout<<"Size of serv_addr == "<<sizeof(serv_addr)<<endl;
  }
  else 
  {
    cout<<"BOUND ON CLIENT SIDE\n"<<endl;
    cout<<"Server Socket on client side == "<<fd<<endl;
  }

  // write(fd, "hello", 5);
}

typedef void * (*connection_handler_t) (void *);

//For the dataserver
NetworkRequestChannel::NetworkRequestChannel(const unsigned short _port_no, connection_handler_t connection_handler, int backlog)
{
  pthread_t handler;
  struct sockaddr_in server_in;
  stringstream ss;
  ss << _port_no;
  string temp= ss.str();
  const char* port = temp.c_str();

  memset(&server_in, 0, sizeof(server_in));
  server_in.sin_family = AF_INET;
  // cout<<"AF_INET run, SERVER SIDE"<<endl;
  server_in.sin_addr.s_addr = htonl(INADDR_ANY);
  // cout<<"INADDR_ANY run, SERVER SIDE"<<endl;
   server_in.sin_port = htons(_port_no);
  // cout<<"sin_port run, SERVER SIDE"<<endl;
  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  
  socklen_t socketSize = sizeof(struct sockaddr_in);

  if((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    cout<<"FAILED: Can't create socket on server side."<<endl;

  if(bind(serverSocket, (struct sockaddr*)&server_in, sizeof(server_in)) < 0)
  {
    cout<<"FAILED: Can't bind on server side..."<<endl;
    // cout<<"Server socket == "<<serverSocket<<endl;
    // cout<<"Size of Server_in"<<sizeof(server_in)<<endl;
  }
  else
    cout<<"BOUND ON SERVER SIDE\n"<<endl;

  cout<<"Listening..."<<endl;
  if(listen(serverSocket, backlog) < 0)
    cout<<"FAILED: Can't listen."<<endl;
  
  while(keep_going)
  {
    int slave = accept(serverSocket, NULL, NULL);
    if(slave < 0)
    {
      cerr<<"Error when trying to accept"<<endl;
    }
    else
      cout<<"Accept was successful."<<endl;
    int * slave2 = new int (slave);
    pthread_create(&handler, NULL, connection_handler, slave2);
    time_t curr_time;
    time(&curr_time);
    char * points = ctime(&curr_time);
  }
  cout<<"connection complete"<<endl;
}

NetworkRequestChannel::~NetworkRequestChannel() 
{
  close(serverSocket);
}

/*--------------------------------------------------------------------------*/
/* READ/WRITE FROM/TO REQUEST CHANNELS  */
/*--------------------------------------------------------------------------*/

const int MAX_MESSAGE = 255;

string NetworkRequestChannel::send_request(string _request) {
  cwrite(_request);
  string s = cread();
  return s;
}

string NetworkRequestChannel::cread() {

  char buf[MAX_MESSAGE];
  memset(buf, 0, sizeof(buf));

  if (read(fd, buf, MAX_MESSAGE) < 0) {
    perror(string("Error reading").c_str());
  }
  
  string s = buf;

  //  cout << "Request Channel (" << my_name << ") reads [" << buf << "]\n";

  return s;

}

int NetworkRequestChannel::cwrite(string _msg) {

  if (_msg.length() >= MAX_MESSAGE) {
    cerr << "Message too long for Channel!\n";
    return -1;
  }

  //  cout << "Request Channel (" << my_name << ") writing [" << _msg << "]";

  const char * s = _msg.c_str();

  if (write(fd, s, strlen(s)+1) < 0) {
    perror(string("Error writing").c_str());
  }

  //  cout << "(" << my_name << ") done writing." << endl;
}

int NetworkRequestChannel::read_fd()
{
  return fd;
}

string NetworkRequestChannel::name() {
  return my_name;
}

string NetworkRequestChannel::server_read(int server_fd) {

  char buf[MAX_MESSAGE];
  memset(buf, 0, sizeof(buf));

  if (read(server_fd, buf, MAX_MESSAGE) < 0) {
    perror(string("Error reading").c_str());
  }
  
  string s = buf;

  //  cout << "Request Channel (" << my_name << ") reads [" << buf << "]\n";

  return s;

}

int NetworkRequestChannel::server_write(int server_fd, string _msg) {

  if (_msg.length() >= MAX_MESSAGE) {
    cerr << "Message too long for Channel!\n";
    return -1;
  }

  //  cout << "Request Channel (" << my_name << ") writing [" << _msg << "]";

  const char * s = _msg.c_str();

  if (write(server_fd, s, strlen(s)+1) < 0) {
    perror(string("Error writing").c_str());
  }

  //  cout << "(" << my_name << ") done writing." << endl;
}

/*--------------------------------------------------------------------------*/
/* ACCESS THE NAME OF REQUEST CHANNEL  */
/*--------------------------------------------------------------------------*/
