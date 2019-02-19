CC=g++
CFLAGS=-O3 -g
LDFLAGS=-Wl,-rpath,/home/user/Bureau/aise/Cours_7/libppm/

TARGET=main
 
all: $(TARGET)

main: main.cpp
	$(CC) $(CFLAGS) $(LDFLAGS) -L../../aise/Cours_7/libppm/ -I../../aise/Cours_7/libppm/ -lppm -fopenmp main.cpp -o $@

clean:
	rm -fr $(TARGET)
