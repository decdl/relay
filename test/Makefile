
CXXFLAGS = -std=c++17 -Wall $C
LDFLAGS = -lboost_system -lpthread $L

all: relayer read_timeout

relayer: relayer/relayer-test

read_timeout: read_timeout/client read_timeout/server

relayer/relayer-test: relayer/relayer-test.cpp ../relayer.cpp ../relay.cpp ../relayer.h ../relay.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@

read_timeout/client: read_timeout/read_timeout-test.cpp ../relay.cpp ../relay.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@

read_timeout/server: read_timeout/test-server.cpp ../relay.cpp ../relay.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@
