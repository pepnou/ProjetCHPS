qcmake_minimum_required (VERSION 3.7.2)
project (Fractale 	VERSION 1.0
					LANGUAGES CXX)


set ( CMAKE_EXPORT_COMPILE_COMMANDS ON )
set ( CMAKE_CXX_COMPILER /gpfslocal/pub/openmpi/1.8.4_gnu47/bin/mpicxx )

set ( EXP_SRC Explo.cpp mandelbrot.cpp color.cpp rdtsc.cpp matOp.cpp workHandler.cpp mpmc.cpp arbre.cpp )
set ( GEN_SRC Gen.cpp mandelbrot.cpp color.cpp rdtsc.cpp matOp.cpp workHandler.cpp mpmc.cpp arbre.cpp )
set ( GMP_INCLUDE /gpfshome/mds/grptraining/training08/include )
#set ( THREAD_INCLUDE /usr/include/c++/7.2.0/thread )
set ( COMPILATION_OPTION "-O0" "-Wall" "-g" "-std=c++11")
set ( OpenCV_INCLUDE /gpfshome/mds/grptraining/training18/include )



add_executable(Explo ${EXP_SRC})
add_executable(Gen ${GEN_SRC})





find_package (OpenMP REQUIRED)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
#include_directories(${OpenMP_INCLUDE_DIRS})
#target_link_libraries (Explo ${OpenMP_LIBRARIES})
#target_link_libraries(Gen ${OpenMP_LIBRARIES})


#find_package (OpenCV REQUIRED)
#include_directories(~/include/opencv)
#target_link_libraries (Explo ${OpenCV_LIBRARIES})
#target_link_libraries(Gen ${OpenCV_LIBRARIES})


find_package (Boost COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries (Explo ${Boost_LIBRARIES})
target_link_libraries(Gen ${Boost_LIBRARIES})


add_library(opencv_core SHARED IMPORTED )
set_target_properties( opencv_core PROPERTIES
	IMPORTED_LOCATION "/gpfshome/mds/grptraining/training18/lib/libopencv_core.so"
	INTERFACE_INCLUDE_DIRECTORIES "/gpfshome/mds/grptraining/training18/include"	
)


add_library(opencv_highgui SHARED IMPORTED )
set_target_properties( opencv_highgui PROPERTIES
        IMPORTED_LOCATION "/gpfshome/mds/grptraining/training18/lib/libopencv_highgui.so"
        INTERFACE_INCLUDE_DIRECTORIES "/gpfshome/mds/grptraining/training18/include"
)


add_library(opencv_imgproc SHARED IMPORTED )
set_target_properties( opencv_imgproc PROPERTIES
        IMPORTED_LOCATION "/gpfshome/mds/grptraining/training18/lib/libopencv_imgproc.so"
        INTERFACE_INCLUDE_DIRECTORIES "/gpfshome/mds/grptraining/training18/include"
)




add_library(gmp STATIC IMPORTED )
set_target_properties( gmp PROPERTIES
        IMPORTED_LOCATION "/gpfshome/mds/grptraining/training08/gmp-6.1.2/.libs/libgmp.a"
	INTERFACE_INCLUDE_DIRECTORIES "/gpfshome/mds/grptraining/training08/include"
)


#target_include_directories ( Explo PUBLIC ${GMP_INCLUDE} ${OpenCV_INCLUDE} )
#target_include_directories ( Gen PUBLIC ${GMP_INCLUDE} ${OpenCV_INCLUDE} )
target_link_libraries ( Explo gmp opencv_core opencv_highgui opencv_imgproc )
target_link_libraries ( Gen gmp opencv_core opencv_highgui opencv_imgproc )


target_compile_options(Explo PUBLIC ${COMPILATION_OPTION})
target_compile_options(Gen PUBLIC ${COMPILATION_OPTION})
