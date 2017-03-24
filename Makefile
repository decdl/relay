
CXXFLAGS = -std=c++17 -Wall $C
LDFLAGS = -lboost_system -lpthread $L

all: relay

relay: main.o relay.o relayer.o client.o server.o
	$(CXX) $(LDFLAGS) $^ -o $@

main.o: main.cpp prec.h.gch
	$(CXX) $(CXXFLAGS) -c main.cpp

relay.o: relay.cpp prec.h.gch
	$(CXX) $(CXXFLAGS) -c relay.cpp

relayer.o: relayer.cpp prec.h.gch
	$(CXX) $(CXXFLAGS) -c relayer.cpp

client.o: client.cpp prec.h.gch
	$(CXX) $(CXXFLAGS) -c client.cpp

server.o: server.cpp prec.h.gch
	$(CXX) $(CXXFLAGS) -c server.cpp

prec.h.gch: prec.h relay.h relayer.h client.h server.h
	$(CXX) $(CXXFLAGS) -c prec.h

clean:
	rm -f relay *.o *.h.gch

strip:
	strip -s relay

