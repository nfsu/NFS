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
	${CMAKE_CURRENT_SOURCE_DIR}/resources/logo.rc
)

target_link_libraries(nfsu nfs)

target_link_libraries(nfsu debug lib/Qt5Cored)
target_link_libraries(nfsu optimized lib/Qt5Core)
target_link_libraries(nfsu debug lib/Qt5Guid)
target_link_libraries(nfsu optimized lib/Qt5Gui)
target_link_libraries(nfsu debug lib/Qt5OpenGLd)
target_link_libraries(nfsu optimized lib/Qt5OpenGL)
target_link_libraries(nfsu debug lib/Qt5Widgetsd)
target_link_libraries(nfsu optimized lib/Qt5Widgets)

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