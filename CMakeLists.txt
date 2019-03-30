cmake_minimum_required (VERSION 3.7.2)
project (Fractale 	VERSION 1.0
					LANGUAGES CXX)


set ( CMAKE_EXPORT_COMPILE_COMMANDS ON )
set ( CMAKE_CXX_COMPILER /usr/bin/mpicxx )

set (EXP_SRC Explo.cpp mandelbrot.cpp color.cpp rdtsc.cpp matOp.cpp workHandler.cpp )
set (GEN_SRC Gen.cpp mandelbrot.cpp color.cpp rdtsc.cpp matOp.cpp workHandler.cpp )

#set (GMP_INCLUDE /usr/local/include/ )
set (THREAD_INCLUDE /usr/include/c++/7.2.0/thread )

set (COMPILATION_OPTION "-O3" "-Wall" "-g")


add_executable(Explo ${EXP_SRC})
add_executable(Gen ${GEN_SRC})





find_path(GMP_INCLUDE_DIR gmp.h
    PATHS ${GMP_PREFIX}/include /usr/include /usr/local/include )

find_library(GMP_LIBRARY NAMES gmp 
    PATHS ${GMP_PREFIX}/lib /usr/lib /usr/local/lib)


if(GMP_INCLUDE_DIR AND GMP_LIBRARY)
    get_filename_component(GMP_LIBRARY_DIR ${GMP_LIBRARY} PATH)
    set(GMP_FOUND TRUE)
endif()

if(GMP_FOUND)
   if(NOT GMP_FIND_QUIETLY)
      MESSAGE(STATUS "Found GMP: ${GMP_LIBRARY}")
   endif()
elseif(GMP_FOUND)
   if(GMP_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find GMP")
   endif()
endif()

target_link_libraries(Explo gmp)
target_link_libraries(Gen gmp)





find_path(MPFR_INCLUDE_DIR mpfr.h
    PATHS ${MPFR_PREFIX}/include /usr/include /usr/local/include )

find_library(MPFR_LIBRARY NAMES mpfr
    PATHS ${MPFR_PREFIX}/lib /usr/lib /usr/local/lib)


if(MPFR_INCLUDE_DIR AND MPFR_LIBRARY)
    get_filename_component(MPFR_LIBRARY_DIR ${MPFR_LIBRARY} PATH)
    set(MPFR_FOUND TRUE)
endif()

if(MPFR_FOUND)
   if(NOT MPFR_FIND_QUIETLY)
      MESSAGE(STATUS "Found MPFR: ${MPFR_LIBRARY}")
   endif()
elseif(MPFR_FOUND)
   if(MPFR_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find MPFR")
   endif()
endif()

target_link_libraries(Explo mpfr )
target_link_libraries(Gen mpfr )







find_package (OpenMP REQUIRED)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")


find_package (OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries (Explo ${OpenCV_LIBRARIES})
target_link_libraries(Gen ${OpenCV_LIBRARIES})


find_package (Boost COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries (Explo ${Boost_LIBRARIES})
target_link_libraries(Gen ${Boost_LIBRARIES})


target_include_directories(Explo PUBLIC THREAD_INCLUDE GMP_INCLUDE)
target_include_directories(Gen PUBLIC THREAD_INCLUDE GMP_INCLUDE)

target_link_libraries (Explo pthread gmp)
target_link_libraries(Gen pthread gmp)


target_compile_options(Explo PUBLIC ${COMPILATION_OPTION})
target_compile_options(Gen PUBLIC ${COMPILATION_OPTION})
