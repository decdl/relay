
CC = c++
CXXFLAGS = -std=c++17 -Wall $C
LDFLAGS = -lboost_system -lpthread $L

all: relay

relay: relay.o relayer.o client.o server.o
	$(CC) $(LDFLAGS) relay.o relayer.o client.o server.o -o relay

relay.o: relay.cpp prec.h.gch
	$(CC) $(CXXFLAGS) -c relay.cpp

relayer.o: relayer.cpp prec.h.gch
	$(CC) $(CXXFLAGS) -c relayer.cpp

client.o: client.cpp prec.h.gch
	$(CC) $(CXXFLAGS) -c client.cpp

server.o: relay_ss.cpp prec.h.gch
	$(CC) $(CXXFLAGS) -c server.cpp

prec.h.gch: prec.h relayer.h #sig.h client.h server.h
	$(CC) $(CXXFLAGS) -c prec.h

clean:
	rm -f relay *.o *.h.gch

strip:
	strip -s relay

