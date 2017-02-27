
CXX = c++
CXX_FLAGS = -std=c++17 -Wall $C
LD_FLAGS = -lboost_system -lpthread $L

all: server client

server: server.cpp
	$(CXX) $(CXX_FLAGS) $(LD_FLAGS) server.cpp -o server

client: client.cpp
	$(CXX) $(CXX_FLAGS) $(LD_FLAGS) client.cpp -o client

clean:
	rm -f server client
