ProjetFractale: main.o mandelbrot.o color.o load.o
	g++ -g -o ProjetFractale main.o mandelbrot.o color.o load.o -Wall -lgmp `pkg-config --cflags --libs opencv`


main.o: main.cpp main.hpp
	g++ -c -g -o main.o main.cpp -Wall -lgmp `pkg-config --cflags --libs opencv`


mandelbrot.o: mandelbrot.cpp mandelbrot.hpp
	g++ -c -g -o mandelbrot.o mandelbrot.cpp -Wall -lgmp `pkg-config --cflags --libs opencv`


color.o: color.cpp color.hpp
	g++ -c -g -o color.o color.cpp -Wall -lgmp `pkg-config --cflags --libs opencv`


load.o: load.cpp load.hpp
	g++ -c -g -o load.o load.cpp -Wall


test: test_img.cpp
	g++ -g -o test test_img.cpp -Wall -lgmp `pkg-config --cflags --libs opencv`


run: ProjetFractale
	./ProjetFractale


runtest: test
	./test


clean:
	rm -f *.o
	rm -f ProjetFractale
	rm -f test
	rm -f alpha.png
	rm -f mandel.png
	rm -f mandelprev.png