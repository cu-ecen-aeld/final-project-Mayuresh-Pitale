CXX ?= g++
CXXFLAGS ?= -Wall -Werror -std=c++17
LDFLAGS ?= -pthread -lrt

all: vibra_app

vibra_app: main.cpp
	$(CXX) $(CXXFLAGS) -o vibra_app main.cpp $(LDFLAGS)

clean:
	rm -f vibra_app
