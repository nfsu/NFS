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

	add_compile_options(
		"$<$<CONFIG:DEBUG>:/MDd>"
		"$<$<CONFIG:RELEASE>:/MD>"
	)

    target_compile_options(nfsu PRIVATE /W4 /WX /MP /wd26812 /wd4201 /EHsc /GR)
else()
    target_compile_options(nfsu PRIVATE -Wall -fms-extensions -Wextra -Werror)
endif()

target_include_directories(nfsu PUBLIC ../nfs/include)
target_include_directories(nfsu PUBLIC include)
target_include_directories(nfsu PUBLIC deps)

target_link_libraries(nfsu nfs)

target_link_libraries(nfsu opengl32)

target_link_libraries(nfsu debug qt5cored)
target_link_libraries(nfsu optimized qt5core)
target_link_libraries(nfsu debug qt5guid)
target_link_libraries(nfsu optimized qt5gui)
target_link_libraries(nfsu debug qt5opengld)
target_link_libraries(nfsu optimized qt5opengl)
target_link_libraries(nfsu debug qt5widgetsd)
target_link_libraries(nfsu optimized qt5widgets)

add_custom_command(TARGET nfsu POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/bin/$<CONFIG> ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>)

add_custom_command(TARGET nfsu POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/resources)