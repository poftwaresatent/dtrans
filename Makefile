CXX= g++
CPPFLAGS= -Wall -I/opt/local/include
CXXFLAGS= $(CPPFLAGS) -pipe -O0 -g
LDFLAGS= -L/opt/local/lib -lpng -lm

SRCS= DistanceTransform.cpp pngio.cpp
OBJS= $(SRCS:.cpp=.o)

all: test pngdtrans

test: $(OBJS) test.o
	$(CXX) -o test test.o $(OBJS) $(LDFLAGS)

pngdtrans: $(OBJS) pngdtrans.o
	$(CXX) -o pngdtrans pngdtrans.o $(OBJS) $(LDFLAGS)

clean:
	rm -f *~ *.o test pngdtrans
