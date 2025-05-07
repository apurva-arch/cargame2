# Multi-Threaded Car Racing Simulation

A C++ standalone desktop application with OpenGL/DirectX rendering, featuring arcade-style physics with exaggerated handling that still requires skill. It supports up to 20 concurrent vehicles and implements basic weather effects like rain and snow that affect handling and visibility.

## Project Overview

This project implements a multi-threaded architecture for a car racing simulation game. The architecture is designed to efficiently utilize multiple CPU cores, provide responsive gameplay with minimal latency, and prevent race conditions between competing threads.

### Key Features

- **Multi-threaded Architecture**: Efficiently utilizes multiple CPU cores
- **Arcade-style Physics**: Exaggerated handling that still requires skill
- **Concurrent Vehicle Management**: Supports up to 20 vehicles simultaneously
- **Dynamic Race Conditions**: Weather effects that impact gameplay
- **AI Driver Behavior**: Distinct personalities and adaptive difficulty
- **Thread-safe Data Structures**: Prevents race conditions between threads

## Project Structure

- `docs/` - Documentation and architecture plans
- `src/` - Source code
  - `core/` - Core engine components (GameEngine, ThreadManager)
  - `physics/` - Physics simulation
  - `rendering/` - Graphics rendering
  - `ai/` - AI driver behavior
  - `audio/` - Sound and music
  - `input/` - User input handling
  - `race/` - Race management
  - `utils/` - Utility classes and functions (ThreadSafeQueue, DoubleBuffer, etc.)
- `assets/` - Game assets (models, textures, sounds)
- `tests/` - Unit and integration tests

## Current Implementation Status

The project currently has the following components implemented:

1. **Core Architecture**
   - Thread management system
   - Game engine with state management
   - Main game loop

2. **Utility Classes**
   - Thread-safe data structures
   - Logging system
   - Profiling tools

3. **Build System**
   - CMake configuration

See [IMPLEMENTATION_PLAN.md](IMPLEMENTATION_PLAN.md) for details on what has been completed and the next steps for implementation.

## Getting Started

### Prerequisites

- C++17 compatible compiler (GCC 8+, Clang 7+, MSVC 2019+)
- CMake 3.14 or higher
- (Future requirements will include OpenGL/DirectX, GLFW, GLM, etc.)

### Building the Project

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

## Development Workflow

1. Review the architecture document at `car_racing_simulation_architecture.md`
2. Check the implementation plan at `IMPLEMENTATION_PLAN.md`
3. Choose a component to implement based on the plan
4. Implement the component following the architecture guidelines
5. Add tests for your implementation
6. Submit a pull request

## Threading Model

The simulation uses multiple threads to handle different aspects:

- **Main Thread**: Game loop coordination, state management
- **Rendering Thread**: OpenGL/DirectX rendering
- **Physics Thread**: Vehicle physics simulation, collision detection
- **AI Thread**: AI driver behavior computation
- **Audio Thread**: Sound effect processing, music playback
- **Input Thread**: Controller/keyboard/mouse input processing
- **Asset Loading Thread**: Dynamic resource loading

Thread synchronization is handled through thread-safe data structures like `ThreadSafeQueue` and `DoubleBuffer`.

## Contributing

Contributions are welcome! Please check the implementation plan and architecture document before starting work on a new feature.

1. Fork the repository
2. Create a feature branch
3. Implement your feature
4. Add tests for your feature
5. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.