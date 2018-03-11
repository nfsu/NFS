include_directories(../nfs/include)
include_directories(include)
include_directories(deps)

file(GLOB_RECURSE nfsu_SRC
	"include/*.h"
	"src/*.c"
	"include/*.hpp"
	"src/*.cpp"
)

add_executable(
	nfsu
	${nfsu_SRC}
)

target_link_libraries(nfsu nfs)

target_link_libraries(nfsu lib/Qt5Core)
target_link_libraries(nfsu lib/Qt5Gui)
target_link_libraries(nfsu lib/Qt5OpenGL)
target_link_libraries(nfsu lib/Qt5Widgets)

add_custom_command(
        TARGET nfsu POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
				${CMAKE_CURRENT_SOURCE_DIR}/bin
				${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
				
add_custom_command(
        TARGET nfsu POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
				${CMAKE_CURRENT_SOURCE_DIR}/resources
				${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/resources)