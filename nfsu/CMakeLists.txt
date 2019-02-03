include_directories(../nfs/include)
include_directories(include)
include_directories(deps)

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
)

target_link_libraries(nfsu nfs opengl32)

target_link_libraries(nfsu debug qt5cored)
target_link_libraries(nfsu optimized qt5core)
target_link_libraries(nfsu debug qt5guid)
target_link_libraries(nfsu optimized qt5gui)
target_link_libraries(nfsu debug qt5opengld)
target_link_libraries(nfsu optimized qt5opengl)
target_link_libraries(nfsu debug qt5widgetsd)
target_link_libraries(nfsu optimized qt5widgets)

add_custom_command(
        TARGET nfsu POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
				${CMAKE_CURRENT_SOURCE_DIR}/bin/$<CONFIG>
				${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>)
				
add_custom_command(
        TARGET nfsu POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
				${CMAKE_CURRENT_SOURCE_DIR}/resources
				${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/resources)