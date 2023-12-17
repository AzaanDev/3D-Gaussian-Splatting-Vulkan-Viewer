include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.10.0
)

FetchContent_GetProperties(googletest)
if (NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
endif ()

enable_testing()

set(TEST_SOURCES
        "test_load_ply.cpp"
        "test_main.cpp"
 "test_projection.cpp")

set(HEADERS
	"${PROJECT_SOURCE_DIR}/gaussian_splatting/includes/gaussian.h"
    "${PROJECT_SOURCE_DIR}/gaussian_splatting/includes/camera.h"
)

set(SOURCES
    "${PROJECT_SOURCE_DIR}/gaussian_splatting/src/camera.cpp"
    "${PROJECT_SOURCE_DIR}/gaussian_splatting/src/gaussian.cpp"

)

add_executable(testing ${TEST_SOURCES} ${SOURCES} ${HEADERS})
find_package(Vulkan REQUIRED)

set_target_properties(testing PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
)

target_include_directories(testing PRIVATE "${PROJECT_SOURCE_DIR}/gaussian_splatting/includes" )
target_link_libraries(testing
        PRIVATE
        gtest
        gtest_main
        glm
)
