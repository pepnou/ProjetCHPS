ProjetFractale: main.o mandelbrot.o color.o
	g++ -g -o ProjetFractale main.o mandelbrot.o color.o -Wall -lgmp `pkg-config --cflags --libs opencv`


main.o: main.cpp main.hpp
	g++ -c -g -o main.o main.cpp -Wall -lgmp `pkg-config --cflags --libs opencv`


mandelbrot.o: mandelbrot.cpp mandelbrot.hpp
	g++ -c -g -o mandelbrot.o mandelbrot.cpp -Wall -lgmp `pkg-config --cflags --libs opencv`


color.o: color.cpp color.hpp
	g++ -c -g -o color.o color.cpp -Wall -lgmp `pkg-config --cflags --libs opencv`


test: test_img.cpp
	g++ -g -o test test_img.cpp -Wall -lgmp `pkg-config --cflags --libs opencv`


runtest: test
	./test


clean:
	rm -f *.o
	rm -f ProjetFractale
	rm -f test
	rm -f alpha.png