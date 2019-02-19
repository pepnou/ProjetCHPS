cmake_minimum_required (VERSION 3.9.0)
project (Fractale 	VERSION 1.0
					DESCRIPTION "generation of interesting view in the mandelbrot set"
					LANGUAGES CXX)

set ( CMAKE_EXPORT_COMPILE_COMMANDS ON )
set ( CMAKE_CXX_COMPILER /usr/bin/mpicxx )

set (SRC main.cpp mandelbrot.cpp color.cpp rdtsc.cpp matOp.cpp mpmc.cpp )
set (TEST_SRC perfTest.cpp rdtsc.cpp )
set (GMP_INCLUDE /usr/local/include/ )
set (THREAD_INCLUDE /usr/include/c++/7.2.0/thread )
set (COMPILATION_OPTION "-O3" "-Wall" "-g")


add_executable(ProjetFractale ${SRC})
add_executable(perfTest ${TEST_SRC})


find_package (OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries (ProjetFractale ${OpenCV_LIBRARIES})
target_link_libraries (perfTest ${OpenCV_LIBRARIES})


find_package (Boost COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries (ProjetFractale ${Boost_LIBRARIES})


target_include_directories(ProjetFractale PUBLIC THREAD_INCLUDE GMP_INCLUDE)
target_include_directories(perfTest PUBLIC THREAD_INCLUDE GMP_INCLUDE)
target_link_libraries (ProjetFractale pthread gmp)
target_link_libraries (perfTest pthread gmp)


# set_target_properties(ProjetFractale PROPERTIES COMPILE_FLAGS ${COMPILATION_OPTION} )
target_compile_options(ProjetFractale PUBLIC ${COMPILATION_OPTION})
target_compile_options(perfTest PUBLIC ${COMPILATION_OPTION})
