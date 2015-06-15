CC=g++
CFLAGS=-std=c++11 -O3 -Wall
LDFLAGS=-lzmqpp -lzmq -lpthread -lsodium

all: peer router

peer: peer.cc
	$(CC) $(CFLAGS) $(LDFLAGS) -o peer.mio peer.cc

router: router.cc
	$(CC) $(CFLAGS) $(LDFLAGS) -o router.mio router.cc

clean:
	rm -rf peer.mio router.mio
