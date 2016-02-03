

#ifndef _NetworkRequestChannel_H_                   // include file only once
#define _NetworkRequestChannel_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <iostream>
#include <fstream>

#include <string>

using namespace std;

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */ 
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CLASS   R e q u e s t C h a n n e l */
/*--------------------------------------------------------------------------*/

class NetworkRequestChannel {

public:

  typedef enum {SERVER_SIDE, CLIENT_SIDE} Side;
 
  typedef enum {READ_MODE, WRITE_MODE} Mode;

private:

  string   my_name;

  Side     my_side;

  int fd;
public:

  /* -- CONSTRUCTOR/DESTRUCTOR */

NetworkRequestChannel(const string _server_host_name, const unsigned short _port_no);
/* Creates a CLIENT-SIDE local copy of the channel. The channel is connected
to the given port number at the given server host.
THIS CONSTRUCTOR IS CALLED BY THE CLIENT. */

NetworkRequestChannel(const unsigned short _port_no, void * (*connection_handler) (void *), int backlog);
/* Creates a SERVER-SIDE local copy of the channel that is accepting connections
at the given port number.
NOTE that multiple clients can be connected to the same server-side end of the
request channel. Whenever a new connection comes in, it is accepted by the
the server, and the given connection handler is invoked. The parameter to
the connection handler is the file descriptor of the slave socket returned
by the accept call.
NOTE that the connection handler does not want to deal with
closing the socket. You will have to close the socket once the
connection handler is done. */

~NetworkRequestChannel();
/* Destructor of the local copy of the channel. */

  string send_request(string _request);
  /* Send a string over the channel and wait for a reply. */

  string cread();
  /* Blocking read of data from the channel. Returns a string of characters
     read from the channel. Returns NULL if read failed. */

  int cwrite(string _msg);
  /* Write the data to the channel. The function returns the number of characters written
     to the channel. */

  static string server_read(int server_fd);

  static int server_write(int server_fd, string _msg);

  string name();
  /* Returns the name of the request channel. */

  int read_fd();
  /* Returns the file descriptor used to read from the channel. */

  int write_fd();
  /* Returns the file descriptor used to write to the channel. */
};


#endif
