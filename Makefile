CXX= g++
CPPFLAGS= -Wall
CXXFLAGS= $(CPPFLAGS) -pipe -O0 -g

SRCS= DistanceTransform.cpp
OBJS= $(SRCS:.cpp=.o)

all: test pngdtrans

test: $(OBJS) test.o
	$(CXX) -o test test.o $(OBJS)

pngdtrans: $(OBJS) pngdtrans.o
	$(CXX) -o pngdtrans pngdtrans.o $(OBJS) -lpng -lm

clean:
	rm -f *~ *.o test pngdtrans
