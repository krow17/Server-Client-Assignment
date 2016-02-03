#include <cassert>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/time.h>

#include <errno.h>
#include <unistd.h>

#include "NetworkRequestChannel.H"
#include "BBuffer.H"

using namespace std;

int requests = 10000; //number of requests
int worker_threads = 20; //
int buffer_size = 500;
string hostname = "127.0.0.1";
unsigned short port = 2020;

//histograms for returned data
vector<int> joe_hist(100);
vector<int> jane_hist(100);
vector<int> john_hist(100);

int read_fds[20];
int write_fds[20];
NetworkRequestChannel * req_chan[20];//array of worker threads

BBuffer * req_buffer; //holds requests
//these are the buffers for statistics
BBuffer * joe_buffer;
BBuffer * jane_buffer;
BBuffer * john_buffer;
int higher_than_other_numbers = 0;


/****REQUEST THREADS PROCESSED HERE*****/
void * req_thread(void * ID)
{
	cout<<"STARTING NEW REQUEST THREAD"<<endl;
	//convert ID to an int
	int id = *((int*)ID);
	for(int i = 0; i < requests; i++)
	{
		Person * person = new Person(id, "___" ); //new person requesting data
		switch(id)
		{	
			//joe
			case 0: person->ident = 0;
					person->response = "data Joe Smith";
					//joe_buffer ->Deposit(*person);
					//cout<<"Joe Smith request"<<endl;
					break;
			//jane
			case 1: person->ident = 1;
					person->response = "data Jane Smith";
					//jane_buffer->Deposit(*person);
					//cout<<"jane smith request"<<endl;
					break;
			//john
			case 2: person->ident = 2;
					person->response = "data John Doe";
					//john_buffer -> Deposit(*person);
					//cout<<"john doe request."<<endl;
					break;
		}
		//add person data request to request buffer
		req_buffer->Deposit(*person);
	}
	cout<<"REQ THREAD DONE"<<endl;
}

//removes a request from BBuffer, which returns a person.
//Conver string response to an int and store that in histogram
void * stat_thread(void * ID)
{
	cout<<"STAT THREAD COMMENCE"<<endl;
	int id = *((int*)ID);
	int count = 0;
	int joe_count = 0;
	cout<<"ID RECIEVED: "<<id<<endl;
	Person person(id, "___");
	for(int i = 0; i < requests; i++)
	{
		switch(id)
		{
			//joe
			case 0: person = joe_buffer->Remove();
					joe_hist.push_back(atoi(person.response.c_str()));
					joe_count++;
					//cout<<joe_count<<" JOE entries so far"<<endl;
					//cout<<"JOE STAT REMOVED"<<endl;
					break;
			//jane
			case 1: person = jane_buffer->Remove();
					jane_hist.push_back(atoi(person.response.c_str()));
					//cout<<"JANE STAT REMOVED"<<endl;
					break;
			//john
			case 2: person = john_buffer->Remove();
					john_hist.push_back(atoi(person.response.c_str()));
					//cout<<"JOHN STAT REMOVED"<<endl;
					count++;
					//cout<<count<<" John entries so far"<<endl;
					break;
		}

	}

	cout<<"STAT THREAD COMPLETED \n"<<endl;

}

void * event_handler_thread(void * in) //event handler
{
	cout<<"Event thread started..."<<endl;
	Person person = Person(0, "___");
	
	int to_select;
	int count = 0;
	int write_count = 0;
	int read_count = 0;

	int who[worker_threads];

	bool done = false;
	fd_set fd;
	struct timeval timeout;
	string to_read;
	
	//usleep(100000);
	  /* -- Start sending a sequence of requests */

	for(int i = 0; i < worker_threads; i++)
	{
		int temp = higher_than_other_numbers;
		req_chan[i] = new NetworkRequestChannel(hostname, port);
		read_fds[i] = req_chan[i]->read_fd();
		if(read_fds[i] > temp)
			higher_than_other_numbers = read_fds[i];
		who[i] = -1;
	}
	cout<<"Request Channels initialized in event thread..."<<endl;
	for(int i = 0; i < worker_threads; i++)
	{	
		person = req_buffer->Remove();
		cout<<"WORKER THREAD: "<<person.response<<endl;
		string resp = person.response;
		who[i] = person.ident;
		req_chan[i]->cwrite(resp);
		cout<<"Response written..."<<endl;
	}

	while(!done)
	{
		// int stat = atoi(req_chan[i]->send_request(resp).c_str());
		// string reply2 = req_chan[i]->send_request(resp);

  		//cout << "Reply to request is '" << reply2 << "'" << endl;
  		FD_ZERO(&fd);
		for(int i = 0; i < worker_threads; i++)
		{
			FD_SET(read_fds[i], &fd);
		}
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		to_select = select(higher_than_other_numbers + 1, &fd, NULL, NULL, NULL);

		if(to_select == -1)
			cout<<"ERROR: Select funtion failed"<<endl;
		if(to_select == 0)
			cout<<"SELECT TIMED OUT"<<endl;
		else
		{
			for(int i = 0; i < worker_threads; i++)
			{
				if(FD_ISSET(req_chan[i]->read_fd(), &fd))
				{
					string to_read = req_chan[i] -> cread();
					read_count++;
					if(who[i] == 0)
					{
						//cout<<"PUSHIING JOE SMITH STAT: "<<to_read<<endl;
						//joe_hist.push_back(atoi(to_read.c_str()));
						joe_buffer->Deposit(Person(0, to_read));
					}

					else if(who[i] == 1)
					{
						//cout<<"PUSHING JANE SMITH STAT: "<<to_read<<endl;
						//jane_hist.push_back(atoi(to_read.c_str()));
						jane_buffer->Deposit(Person(1, to_read));
					}

					else if(who[i] == 2)
					{
						//cout<<"PUSHING JOHN DOE STAT: "<<to_read<<endl;
						//john_hist.push_back(atoi(to_read.c_str()));
						john_buffer->Deposit(Person(2, to_read));
					}

					if(write_count < requests*3)
					{
						person = req_buffer->Remove();
						write_count++;
						string resp = person.response;
						who[i] = person.ident;
						req_chan[i] -> cwrite(resp);
					}

				}
			}
		}

		if(read_count == requests*3 - 100)
		{
			break;
		}

	}
	for(int i=0; i<worker_threads; i++)
    {
       	string reply = req_chan[i]->send_request("quit");
        cout << "Reply to request 'quit' is '" << reply << "'" << endl;
    }
}

void show_histo(vector<int> hist, string who)
{
	int count = 0;
	cout<<who<<"'s histogram:"<<endl;
	for(int i = 0; i < hist.size(); i++)
	{
		count++;
		cout<<"( "<<hist[i]<<" ) ";
	}
	cout<<count<<" ENTRIES"<<endl;
}

/****MAIN FUNCTION****/
int main(int argc, char * argv[])
{
	int input = 0;
	while((input = getopt(argc, argv, "n:w:b:"))!= -1)
	{
		if(input == 'n')
			requests = atoi(optarg);
		if(input == 'w')
			worker_threads = atoi(optarg);
		if(input == 'b')
			buffer_size = atoi(optarg);
		else
			cout<<"you messed up"<<endl;
	}

	pthread_t req_t[3];
	pthread_t stat_t[3];
	pthread_t go;

	pthread_mutex_t m;
	pthread_mutex_init(&m, NULL);
	//because pthread_create forces me to be creative and is stupid. Stupid void pointers
	int * joe = new int(0);
	int * jane = new int(1);
	int * john = new int(2);

	req_buffer = new BBuffer(buffer_size);
	joe_buffer = new BBuffer(buffer_size);
	jane_buffer = new BBuffer(buffer_size);
	john_buffer = new BBuffer(buffer_size);
	void * in;

	// pid_t pid = fork();
	// if(pid == 0)//child process, launch dataserver
	// {
	// 	execl("./dataserver2", NULL, NULL);
	// }

	// else if(pid > 0)
	// {
		cout<<"CLIENT STARTED:" << endl;
        cout << "done." << endl;
        timeval begin, end;
		gettimeofday(&begin, NULL);

		cout << "Establishing control channel... " << flush;
	
		pthread_create(&req_t[0], NULL, req_thread, joe);
		pthread_create(&req_t[1], NULL, req_thread, jane);
		pthread_create(&req_t[2], NULL, req_thread, john);
		//sleep(1);
		pthread_create(&go, NULL, event_handler_thread, NULL);
		//sleep(1);
		pthread_create(&stat_t[0], NULL, stat_thread, joe);
        pthread_create(&stat_t[1], NULL, stat_thread, jane);
        pthread_create(&stat_t[2], NULL, stat_thread, john);


        // pthread_join(req_t[0], NULL);
        // pthread_join(req_t[1], NULL);
        // pthread_join(req_t[2], NULL);
        // pthread_join(stat_t[0], NULL);
        // pthread_join(stat_t[1], NULL);
        // pthread_join(stat_t[2], NULL);
        // pthread_join(go, NULL);
 
        sleep(6);
        // usleep(5);


         gettimeofday(&end, NULL);

        show_histo(joe_hist, "Joe Smith");
        cout<<'\n'<<endl;
        show_histo(jane_hist, "Jane Smith");
        cout<<'\n'<<endl;
        show_histo(john_hist, "John Doe");
        cout<<"Total request time: "<<end.tv_sec-begin.tv_sec<<" sec "<<end.tv_usec-begin.tv_usec<<" musec"<<endl;


      






		
		//cout<<"Establishing control channel... "<<flush;
		//NetworkRequestChannel chan("control", NetworkRequestChannel::CLIENT_SIDE);
		//cout<<"done."<<endl;
	
}
