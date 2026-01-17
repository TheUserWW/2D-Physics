# 2D Physics

A real-time 2D physics simulation engine built with C++ and OpenGL, featuring gravitational and electric field simulations with interactive object manipulation.

## Features

- **Real-time Physics Simulation**: Accurate gravitational and electric field calculations
- **Interactive Object Creation**: Create circles with customizable properties
- **Object Manipulation**: Drag and drop objects with real-time physics response
- **Visual Feedback**: Real-time velocity and acceleration visualization
- **ImGui Interface**: User-friendly interface for controlling simulation parameters
- **Cross-platform Support**: Windows-compatible with OpenGL rendering

## Project Structure

```bash
2DPhysics/
├── src/
│   ├── main.cpp          # Main application and rendering loop
├── include/
│   ├── axioms.h          # Physics constants and field definitions
│   ├── Circle.h          # Circle object implementation
│   ├── polygon.h         # Polygon object implementation
├── Dependencies/         # External libraries (GLEW, GLFW, ImGui)
├── CMakeLists.txt        # CMake build configuration
└── 2DPhysics.rc          # Windows resource file
```


## Physics Engine

The simulation implements realistic physics including:

- **Gravitational Fields**: Objects experience gravitational forces based on mass
- **Electric Fields**: Charged objects interact with electric fields
- **Newtonian Mechanics**: Velocity, acceleration, and force calculations
- **Collision Detection**: Basic collision handling between objects

## Object Types

### Circle Objects
- Customizable radius and mass
- Real-time physics simulation
- Visual representation with smooth rendering
- Charge properties for electric field interactions

### Polygon Objects
- Multi-sided shape support
- Physics simulation capabilities
- Customizable properties

## Building the Project

### Prerequisites
- CMake 3.10+
- C++23 compatible compiler
- OpenGL development libraries
- Windows SDK (for Windows builds)

### Build Instructions
```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the project
cmake --build . --config Release
```

## Usage

1. **Run the application**: Launch the executable to start the simulation
2. **Create Objects**: Use the interface to create circles with desired properties
3. **Adjust Fields**: Modify gravitational and electric field parameters
4. **Interact**: Drag objects to apply forces and observe physics behavior
5. **Observe**: Watch realistic physics interactions in real-time

## Controls

- **Mouse Drag**: Select and move objects
- **UI Controls**: Adjust simulation parameters through the ImGui interface
- **Object Creation**: Use the creation panel to add new objects to the simulation

## Technical Details

- **Rendering**: OpenGL 3.3+ with immediate mode rendering
- **Physics**: Real-time numerical integration
- **UI**: ImGui for interactive controls
- **Window Management**: GLFW for cross-platform window handling
- **Extensions**: GLEW for OpenGL extension loading

## Dependencies

- **GLFW**: Download link: https://www.glfw.org/download.html
- **GLEW**: Download link: https://glew.sourceforge.net/
- **ImGui**: Project link: https://github.com/ocornut/imgui
