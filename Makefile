CXX= g++
CPPFLAGS= -Wall -I/opt/local/include
CXXFLAGS= $(CPPFLAGS) -pipe -O0 -g
LDFLAGS= -L/opt/local/lib -lpng -lm

SRCS= DistanceTransform.cpp pngio.cpp
OBJS= $(SRCS:.cpp=.o)

all: test pngdtrans

# Building gdtrans is painful on OS X, at least on my MacBook, because
# the macports fltk expects arch=i386 but libpng wants arch=x86_64 and
# then Apple's ld bails with a funky error message. On Linux it is
# much easier... just replace the above target line with this one. On
# Linux, you can also ditch the '/opt/local/include' parts from
# CPPFLAGS and LDFLAGS, but it shouldn't hurt to keep them.
#
# all: test pngdtrans gdtrans

test: $(OBJS) test.o
	$(CXX) -o test test.o $(OBJS) $(LDFLAGS)

pngdtrans: $(OBJS) pngdtrans.o
	$(CXX) -o pngdtrans pngdtrans.o $(OBJS) $(LDFLAGS)

gdtrans: $(OBJS) gdtrans.o
	$(CXX) -o gdtrans gdtrans.o $(OBJS) $(LDFLAGS) `fltk-config --ldflags`

gdtrans.o: gdtrans.cpp
	$(CXX) `fltk-config --cxxflags` $(CXXFLAGS) -c gdtrans.cpp

clean:
	rm -f *~ *.o test pngdtrans gdtrans
