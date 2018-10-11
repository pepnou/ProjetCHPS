ProjetFractale: main.o mandelbrot.o
	g++ -g -o ProjetFractale main.o mandelbrot.o -Wall


main.o: main.cpp main.hpp
	g++ -c -g -o main.o main.cpp -Wall


mandelbrot.o: mandelbrot.cpp mandelbrot.hpp
	g++ -c -g -o mandelbrot.o mandelbrot.cpp -Wall


clean:
	rm -f *.o
	rm -f ProjetFractale