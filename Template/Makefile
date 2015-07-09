CC=g++
CFLAGS=-std=c++11 -O3 -Wall
LDFLAGS=-lzmqpp -lzmq -lpthread -lsodium

all: chat_client.o chat_server.o

chat_client.o: chat_client.cpp
	$(CC) -o chat_client.o chat_client.cpp $(CFLAGS) $(LDFLAGS)

chat_server.o: chat_server.cpp
	$(CC) -o chat_server.o chat_server.cpp $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf *.o
