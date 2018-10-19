cmake_minimum_required (VERSION 3.9.0)
project (Fractale 	VERSION 1.0
					DESCRIPTION "generation of interesting view in the mandelbrot set"
					LANGUAGES CXX)





set (SRC main.cpp mandelbrot.cpp color.cpp load.cpp rdtsc.cpp)
set (LIB_PATH /usr/include/c++/7.2.0/thread /usr/local/include/)



add_executable(ProjetFractale ${SRC})

find_package (OpenCV)
if (OpenCV_FOUND)
  include_directories(${OpenCV_INCLUDE_DIRS})
  target_link_libraries (ProjetFractale ${OpenCV_LIBRARIES})
endif (OpenCV_FOUND)

target_include_directories(ProjetFractale PUBLIC LIB_PATH)
target_link_libraries (ProjetFractale pthread gmp)


