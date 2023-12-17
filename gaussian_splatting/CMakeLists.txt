set(HEADERS
	"includes/camera.h"
     
	"includes/gaussian.h"
	"includes/application.h"
	"includes/vertex.h" 
	"includes/uniform.h"
 )

set(SOURCES
	main.cpp
	"src/camera.cpp"
	"src/gaussian.cpp" 
	"src/application.cpp"
)

add_executable("gaussian_splatting" ${SOURCES} ${HEADERS})
find_package(Vulkan REQUIRED)
target_include_directories(gaussian_splatting PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/includes)
target_include_directories("gaussian_splatting" PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(gaussian_splatting PRIVATE glm Vulkan::Vulkan glfw)
