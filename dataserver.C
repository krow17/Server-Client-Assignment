
#include <cassert>
#include <cstring>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include "NetworkRequestChannel.H"

using namespace std;

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* VARIABLES */
/*--------------------------------------------------------------------------*/

static int nthreads = 0;
int MAX_MSG = 255;

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- SUPPORT FUNCTIONS */
/*--------------------------------------------------------------------------*/

string int2string(int number) {
   stringstream ss;//create a stringstream
   ss << number;//add number to the stream
   return ss.str();//return a string with the contents of the stream
}

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- THREAD FUNCTIONS */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- INDIVIDUAL REQUESTS */
/*--------------------------------------------------------------------------*/


void process_hello(int file_descriptor, const string & _request) {
  NetworkRequestChannel::server_write(file_descriptor, "hello to you too");
}

void process_data(int file_descriptor, const string &  _request) {
  usleep(1000 + (rand() % 5000));
  //_channel.cwrite("here comes data about " + _request.substr(4) + ": " + int2string(random() % 100));
  NetworkRequestChannel::server_write(file_descriptor, int2string(rand() % 100));
}


/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- THE PROCESS REQUEST LOOP */
/*--------------------------------------------------------------------------*/

void process_request(int file_descriptor, const string & _request) {

  if (_request.compare(0, 5, "hello") == 0) {
    process_hello(file_descriptor, _request);
  }
  else if (_request.compare(0, 4, "data") == 0) {
    process_data(file_descriptor, _request);
  }
  else {
    cout<<"FAILED: processing request on server side."<<endl;
  }

}
void * connection_handler(void * arg)
{
	int * file_descriptor = (int*)arg;

	while(1)
	{
		string request = NetworkRequestChannel::server_read(*file_descriptor);

		if (request.compare("quit") == 0)
		{
			NetworkRequestChannel::server_write(*file_descriptor, "bye");
			usleep(8000);
			break;
		}

		process_request(*file_descriptor, request);
	}
	cout<<"Connection Closed\n";

}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {

	int backlog = 100;
	unsigned short portn = 2020;
	unsigned short pN = 0;
	int blog = 0;
  //  cout << "Establishing control channel... " << flush;
  NetworkRequestChannel control_channel(portn, connection_handler, backlog);
  //  cout << "done.\n" << flush;
  control_channel.~NetworkRequestChannel();

}
