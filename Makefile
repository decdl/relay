
CC = c++
CXXFLAGS = -std=c++17 -Wall $C
LDFLAGS = -lboost_system -lpthread $L

all: relay

relay: main.o relay.o relayer.o client.o server.o
	$(CC) $(LDFLAGS) main.o relay.o relayer.o client.o server.o -o relay

main.o: main.cpp prec.h.gch
	$(CC) $(CXXFLAGS) -c main.cpp

relay.o: relay.cpp prec.h.gch
	$(CC) $(CXXFLAGS) -c relay.cpp

relayer.o: relayer.cpp prec.h.gch
	$(CC) $(CXXFLAGS) -c relayer.cpp

client.o: client.cpp prec.h.gch
	$(CC) $(CXXFLAGS) -c client.cpp

server.o: server.cpp prec.h.gch
	$(CC) $(CXXFLAGS) -c server.cpp

prec.h.gch: prec.h relay.h relayer.h client.h server.h
	$(CC) $(CXXFLAGS) -c prec.h

clean:
	rm -f relay *.o *.h.gch

strip:
	strip -s relay

