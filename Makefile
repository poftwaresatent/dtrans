CXX= g++
CPPFLAGS= -Wall -I/opt/local/include
CXXFLAGS= $(CPPFLAGS) -pipe -O0 -g
LDFLAGS= -L/opt/local/lib -lpng -lm

SRCS= DistanceTransform.cpp pngio.cpp
OBJS= $(SRCS:.cpp=.o)

all: test pngdtrans
# gdtrans

test: $(OBJS) test.o
	$(CXX) -o test test.o $(OBJS) $(LDFLAGS)

pngdtrans: $(OBJS) pngdtrans.o
	$(CXX) -o pngdtrans pngdtrans.o $(OBJS) $(LDFLAGS)

gdtrans: $(OBJS) gdtrans.o
	$(CXX) -o gdtrans gdtrans.o $(OBJS) $(LDFLAGS) `fltk-config --ldflags`

gdtrans.o: gdtrans.cpp
	$(CXX) `fltk-config --cxxflags` $(CXXFLAGS) -c gdtrans.cpp

clean:
	rm -f *~ *.o test pngdtrans
