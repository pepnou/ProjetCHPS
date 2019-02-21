CC=gcc
CFLAGS=-O3 -g
LDFLAGS=-Wl,-rpath,$(PWD) 

TARGET=test
 
all: $(TARGET)

libppm.so : ppm.c ppm.h
	$(CC) $(CFLAGS)  -fpic -shared $^ -o $@

test: test.c libppm.so
	$(CC) $(CFLAGS) $(LDFLAGS) -lppm -L. test.c -o $@

clean:
	rm -fr $(TARGET) *.so
