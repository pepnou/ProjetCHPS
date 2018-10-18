ProjetFractale: main.o mandelbrot.o color.o load.o rdtsc.o
	g++ -g -o ProjetFractale main.o mandelbrot.o color.o load.o rdtsc.o -Wall -lgmp `pkg-config --cflags --libs opencv` -lpthread


main.o: main.cpp main.hpp
	g++ -c -g -o main.o main.cpp -Wall -lgmp `pkg-config --cflags --libs opencv`


mandelbrot.o: mandelbrot.cpp mandelbrot.hpp
	g++ -c -g -o mandelbrot.o mandelbrot.cpp -Wall -lgmp `pkg-config --cflags --libs opencv` -lpthread


color.o: color.cpp color.hpp
	g++ -c -g -o color.o color.cpp -Wall -lgmp `pkg-config --cflags --libs opencv`


load.o: load.cpp load.hpp
	g++ -c -g -o load.o load.cpp -Wall


rdtsc.o: rdtsc.cpp rdtsc.hpp
	g++ -c -g -o rdtsc.o rdtsc.cpp -Wall


test: test_img.cpp
	g++ -g -o test test_img.cpp -Wall -lgmp `pkg-config --cflags --libs opencv`


run: ProjetFractale
	./ProjetFractale
	
install: install
		sudo apt-get update
		sudo apt-get install libgmp3-dev
		sudo apt-get install libopencv-dev


runtest: test
	./test
	
#sudo apt-get install libgmp3-dev
#sudo apt-get install libopencv-dev

clean:
	rm -f *.o
	rm -f ProjetFractale
	rm -f test
	rm -f alpha.png
	rm -f mandel.png
	rm -f mandelprev.png
