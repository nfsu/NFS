cmake_minimum_required(VERSION 3.13.0)
project(nfsu)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE nfsu_SRC
	"include/*.h"
	"src/*.c"
	"include/*.hpp"
	"src/*.cpp"
)

link_directories(lib)

add_executable(
	nfsu
	${nfsu_SRC}
	${CMAKE_CURRENT_SOURCE_DIR}/resources/logo.rc
	CMakeLists.txt
)

if(MSVC)
    target_compile_options(nfsu PRIVATE /W4 /WX /MD /MP /wd26812 /wd4201 /EHsc /GR)
else()
    target_compile_options(nfsu PRIVATE -Wall -fms-extensions -Wextra -Werror)
endif()

target_include_directories(nfsu PUBLIC ../nfs/include)
target_include_directories(nfsu PUBLIC include)
target_include_directories(nfsu PUBLIC deps)

target_link_libraries(nfsu PUBLIC nfs)

target_link_libraries(nfsu PRIVATE opengl32)

target_link_libraries(nfsu PRIVATE debug qt5cored qt5guid qt5opengld qt5widgetsd)
target_link_libraries(nfsu PRIVATE optimized qt5core qt5gui qt5opengl qt5widgets)

add_custom_command(TARGET nfsu POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/bin/$<CONFIG> ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>)

add_custom_command(TARGET nfsu POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/resources)