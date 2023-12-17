cmake_minimum_required (VERSION 3.26)

set(PROJECT_NAME "GaussianSplatting")

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project (${PROJECT_NAME} LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build tests" ON)

# Include sub-projects.
add_subdirectory ("gaussian_splatting")
add_subdirectory(vendors)


if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

### ADD SHADER FILES