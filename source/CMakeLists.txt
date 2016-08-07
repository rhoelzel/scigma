Cmake_minimum_required(VERSION 3.0)
project(scigma)

enable_language(Fortran)

add_subdirectory(external)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(CMAKE_MACOSX_RPATH OFF)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Werror -Weverything -Wno-c++98-compat-pedantic -Wno-documentation -Wno-unknown-pragmas")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wpedantic -Wextra -Werror -Wno-unknown-pragmas -pthread")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pthread")
	  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
	  endif()

add_subdirectory(common)
add_subdirectory(num)
add_subdirectory(gui)
add_subdirectory(dat)

file(WRITE empty.cpp "")
add_library(scigma SHARED empty.cpp) 

target_link_libraries(scigma "-Wl,-all_load"
			     gui num dat common
			     lapack odessa tinythread
			     libpng zlib glew glfw
			     anttweakbar ${GLFW_LIBRARIES})

