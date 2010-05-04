CXX= g++
CPPFLAGS= -Wall
CXXFLAGS= $(CPPFLAGS) -pipe -O0 -g

SRCS= DistanceTransform.cpp
OBJS= $(SRCS:.cpp=.o)

all: test

test: $(OBJS) test.o
	$(CXX) -o test test.o $(OBJS)

clean:
	rm -f *~ *.o test
