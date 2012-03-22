CC= gcc
CPPFLAGS= -Wall -I/opt/local/include
CFLAGS= $(CPPFLAGS) -pipe -O0 -g

all: heap_test usdtrans

usdtrans: usdtrans.o heap.o
	$(CC) -o usdtrans usdtrans.o heap.o

heap_test: heap.o heap_test.o
	$(CC) -o heap_test heap_test.o heap.o

clean:
	rm -f *~ *.o heap_test usdtrans

heap.o: heap.h
heap_test.o: heap.h
