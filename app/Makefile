CXX ?= g++
CXXFLAGS ?= -Wall -std=c++17
LDFLAGS ?= -pthread -lrt -ldl

all: vibra_app

vibra_app: main.cpp
	$(CXX) $(CXXFLAGS) -o vibra_app main.cpp $(LDFLAGS)

clean:
	rm -f vibra_app
