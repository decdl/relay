
CC = c++
CXXFLAGS = -std=c++17 -Wall $F
LDFLAGS = $F

all: proxy client server

proxy: proxy.o
	$(CC) $(LDFLAGS) -lboost_system proxy.o -o proxy

client: client.o
	$(CC) $(LDFLAGS) client.o -o client

server: server.o
	$(CC) $(LDFLAGS) server.o -o server

proxy.o: proxy.cpp
	$(CC) $(CXXFLAGS) -c proxy.cpp -o proxy.o

client.o: client.cpp
	$(CC) $(CXXFLAGS) -c client.cpp -o client.o

server.o: server.cpp
	$(CC) $(CXXFLAGS) -c server.cpp -o server.o

strip:
	strip -s proxy
	strip -s client
	strip -s server

clean:
	rm -f *.o proxy client server
