# Build Instructions

## Prerequisites

- C++17 compatible compiler (GCC 8+, Clang 7+, MSVC 2019+)
- CMake 3.14 or higher
- OpenGL or DirectX development libraries
- GLFW or similar windowing library
- GLM for mathematics
- Assimp for model loading
- OpenAL for audio

## Building on Windows

1. Clone the repository
   ```
   git clone https://github.com/yourusername/car-racing-simulation.git
   cd car-racing-simulation
   ```

2. Create a build directory
   ```
   mkdir build
   cd build
   ```

3. Configure with CMake
   ```
   cmake ..
   ```

4. Build the project
   ```
   cmake --build . --config Release
   ```

5. Run the executable
   ```
   ./bin/CarRacingSimulation
   ```

## Building on Linux

1. Install dependencies
   ```
   sudo apt-get install build-essential libglfw3-dev libglm-dev libassimp-dev libopenal-dev
   ```

2. Clone the repository
   ```
   git clone https://github.com/yourusername/car-racing-simulation.git
   cd car-racing-simulation
   ```

3. Create a build directory
   ```
   mkdir build
   cd build
   ```

4. Configure with CMake
   ```
   cmake ..
   ```

5. Build the project
   ```
   make
   ```

6. Run the executable
   ```
   ./bin/CarRacingSimulation
   ```

## CMake Configuration

The project uses CMake for build configuration. The main CMakeLists.txt would include:

```cmake
cmake_minimum_required(VERSION 3.14)
project(CarRacingSimulation VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(assimp REQUIRED)
find_package(OpenAL REQUIRED)

# Add subdirectories
add_subdirectory(src)
add_subdirectory(tests)

# Main executable
add_executable(CarRacingSimulation src/main.cpp)

# Link libraries
target_link_libraries(CarRacingSimulation
    PRIVATE
    core
    physics
    rendering
    ai
    audio
    input
    race
    utils
    ${OPENGL_LIBRARIES}
    glfw
    glm
    assimp
    OpenAL
)
```

## Project Structure

Each subdirectory in the `src` folder would have its own CMakeLists.txt file to define libraries and dependencies.

## Testing

To build and run tests:

```
cd build
cmake .. -DBUILD_TESTING=ON
cmake --build .
ctest
```

## Performance Profiling

The project includes performance profiling tools. To enable profiling:

```
cmake .. -DENABLE_PROFILING=ON