cmake_minimum_required(VERSION 3.14)
project(CarRacingSimulation VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Enable profiling in debug mode
option(ENABLE_PROFILING "Enable performance profiling" ON)
if(ENABLE_PROFILING)
    add_compile_definitions(ENABLE_PROFILING)
endif()

# Platform-specific settings
if(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0601)
    if(MSVC)
        # MSVC-specific compiler flags
        add_compile_options(/W4 /MP)
    endif()
else()
    # GCC/Clang compiler flags
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Source files
set(UTILS_SOURCES
    # Header-only libraries, no source files needed
)

set(CORE_SOURCES
    src/core/ThreadManager.cpp
    src/core/GameEngine.cpp
)

set(PHYSICS_SOURCES
    # Will be implemented later
)

set(RENDERING_SOURCES
    # Will be implemented later
)

set(AI_SOURCES
    # Will be implemented later
)

set(AUDIO_SOURCES
    # Will be implemented later
)

set(INPUT_SOURCES
    # Will be implemented later
)

set(RACE_SOURCES
    # Will be implemented later
)

# Main executable
add_executable(CarRacingSimulation
    src/main.cpp
    ${UTILS_SOURCES}
    ${CORE_SOURCES}
    ${PHYSICS_SOURCES}
    ${RENDERING_SOURCES}
    ${AI_SOURCES}
    ${AUDIO_SOURCES}
    ${INPUT_SOURCES}
    ${RACE_SOURCES}
)

# Find required packages
# These will be uncommented as we implement the respective subsystems
# find_package(OpenGL REQUIRED)
# find_package(glfw3 REQUIRED)
# find_package(glm REQUIRED)
# find_package(assimp REQUIRED)
# find_package(OpenAL REQUIRED)

# Link libraries
target_link_libraries(CarRacingSimulation
    PRIVATE
    # ${OPENGL_LIBRARIES}
    # glfw
    # glm
    # assimp
    # OpenAL
)

# Threading support
find_package(Threads REQUIRED)
target_link_libraries(CarRacingSimulation PRIVATE Threads::Threads)

# Install targets
install(TARGETS CarRacingSimulation
    RUNTIME DESTINATION bin
)

# Testing
option(BUILD_TESTING "Build tests" OFF)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "CarRacingSimulation Configuration:")
message(STATUS "  CMake version: ${CMAKE_VERSION}")
message(STATUS "  C++ Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Profiling: ${ENABLE_PROFILING}")
message(STATUS "  Testing: ${BUILD_TESTING}")
message(STATUS "")