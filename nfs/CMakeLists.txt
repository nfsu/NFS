cmake_minimum_required(VERSION 3.13.0)
project(nfs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
	set(CMAKE_CXX_FLAGS "/W4 /WX /MD /MP /wd26812 /wd4201 /EHsc /GR")
else()
	set(CMAKE_CXX_FLAGS "-Wall -Wpedantic -Wextra -Werror")
endif()

file(GLOB nfs_SRC
	"include/*.h"
	"src/*.c"
	"include/*.hpp"
	"src/*.cpp"
)

add_library(
	nfs STATIC
	${nfs_SRC}
)

target_include_directories(nfs PUBLIC include)