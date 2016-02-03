# makefile

all: dataserver client

NetworkRequestChannel.o: NetworkRequestChannel.H NetworkRequestChannel.C
	g++ -c -g NetworkRequestChannel.C

semaphore.o: semaphore.H semaphore.C
	g++ -c -g semaphore.C

dataserver: dataserver.C NetworkRequestChannel.o 
	g++ -g -o dataserver dataserver.C NetworkRequestChannel.o -lpthread

client: client.C NetworkRequestChannel.o client.C semaphore.o BBuffer.H
	g++ -g -o client client.C semaphore.o NetworkRequestChannel.o -lpthread

run:
	./dataserver -b 100 -p 6258
