
CC = c++
CXXFLAGS = -std=c++17 -Wall $C
LDFLAGS = -lboost_system -lpthread $L

all: relay

relay: relay.o relay_cc.o relay_ss.o
	$(CC) $(LDFLAGS) relay.o relay_cc.o relay_ss.o -o relay

relay_cc.o: relay_cc.cpp prec.h.gch
	$(CC) $(CXXFLAGS) -c relay_cc.cpp

relay_ss.o: relay_ss.cpp prec.h.gch
	$(CC) $(CXXFLAGS) -c relay_ss.cpp

prec.h.gch: prec.h relay_cc.h relay_ss.h
	$(CC) $(CXXFLAGS) -c prec.h

clean:
	rm -f relay-cc relay-ss *.o *.h.gch

strip:
	strip -s relay

