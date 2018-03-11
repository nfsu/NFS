file(GLOB nfs_SRC
	"include/*.h"
	"src/*.c"
	"include/*.hpp"
	"src/*.cpp"
)

include_directories(include)

add_library(
	nfs STATIC
	${nfs_SRC}
)