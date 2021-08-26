cmake_minimum_required(VERSION 3.13.0)
project(nfs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB nfs_SRC
	"include/*.h"
	"src/*.c"
	"include/*.hpp"
	"src/*.cpp"
)

add_library(
	nfs STATIC
	${nfs_SRC}
	CMakeLists.txt
)

if(MSVC)

	add_compile_options(
		"$<$<CONFIG:DEBUG>:/MDd>"
		"$<$<CONFIG:RELEASE>:/MD>"
	)

    target_compile_options(nfs PRIVATE /W4 /WX /MP /wd26812 /wd4201 /EHsc /GR)
else()
    target_compile_options(nfs PRIVATE -Wall -fms-extensions -Wextra -Werror)
endif()

target_include_directories(nfs PUBLIC include)