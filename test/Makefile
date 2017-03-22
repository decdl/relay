
CXXFLAGS = -std=c++17 -Wall $C
LDFLAGS = -lboost_system -lpthread $L

all: relayer-test

relayer-test: relayer-test.cpp ../relayer.cpp ../relay.cpp ../relayer.h ../relay.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) relayer-test.cpp ../relayer.cpp ../relay.cpp -o $@
