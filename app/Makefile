CXX ?= g++
CXXFLAGS ?= -Wall -std=c++17
# Added -ltensorflow-lite and -ldl
LDFLAGS ?= -pthread -lrt -ltensorflow-lite -ldl

all: vibra_app

vibra_app: main.cpp
	$(CXX) $(CXXFLAGS) -o vibra_app main.cpp $(LDFLAGS)

clean:
	rm -f vibra_app
