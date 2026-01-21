# 2D Physics

A real-time 2D physics simulation engine built with C++ and OpenGL, featuring gravitational and electric field simulations with interactive object manipulation.
<img width="1624" height="947" alt="image" src="https://github.com/user-attachments/assets/99867237-874f-4416-9fb7-ce4e8a8b8d61" />

## Features

- **Real-time Physics Simulation**: Accurate gravitational and electric field calculations
- **Interactive Object Creation**: Create circles with customizable properties
- **Object Manipulation**: Drag and drop objects with real-time physics response
- **Visual Feedback**: Real-time velocity and acceleration visualization
- **ImGui Interface**: User-friendly interface for controlling simulation parameters
- **Cross-platform Support**: Windows-compatible with OpenGL rendering
- **Time Control**: Adjustable time scale for slowing down or speeding up simulations
- **Vertical Sync Control**: Toggle vertical sync for optimal performance
- **Object Management**: Create, modify, and delete objects in real-time
- **Field Direction Controls**: Precise control over gravitational and electric field directions
- **Charge Presets**: Quick charge value selection for electric field interactions
- **Velocity Limiting**: Maximum velocity constraints for realistic object movement

## Project Structure

```bash
2DPhysics/
├── src/
│   ├── main.cpp          # Main application and rendering loop
├── include/
│   ├── axioms.h          # Physics constants and field definitions
│   ├── Circle.h          # Circle object implementation
│   ├── polygon.h         # Polygon object implementation
├── Dependencies/         # External libraries (GLEW, GLFW, ImGui, cPhysics)
├── CMakeLists.txt        # CMake build configuration
├── 2DPhysics.rc          # Windows resource file
└── icon.ico              # Application icon
```


## Physics Engine

The simulation implements realistic physics including:

- **Gravitational Fields**: Objects experience gravitational forces based on mass
- **Electric Fields**: Charged objects interact with electric fields
- **Newtonian Mechanics**: Velocity, acceleration, and force calculations
- **Collision Detection**: Basic collision handling between objects
- **Universal Gravitation**: Realistic gravitational interactions between all objects
- **Coulomb's Law**: Electric force calculations between charged objects
- **Field Superposition**: Combined effects of multiple fields
- **Mass and Charge Properties**: Objects can have both mass and charge for multi-field interactions

## Object Types

### Circle Objects
- Customizable radius and mass
- Real-time physics simulation
- Visual representation with smooth rendering
- Charge properties for electric field interactions
- Adjustable movement status (fixed or movable)

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
2. **Create Objects**: Use the creation tools to add circles with customizable properties
3. **Adjust Fields**: Modify gravitational and electric field parameters and directions
4. **Interact**: Drag objects to apply forces and observe physics behavior
5. **Manage Objects**: Modify object properties or delete objects as needed
6. **Control Simulation**: Adjust time scale and toggle vertical sync for optimal performance
7. **Observe**: Watch realistic physics interactions in real-time

## Controls

- **Mouse Drag**: Select and move objects with velocity transfer
- **Left Click**: Create new circles in creation mode
- **UI Controls**: Adjust simulation parameters through the ImGui interface
- **Object Creation**: Use the creation panel to add new objects with customizable properties
- **Field Controls**: Modify field magnitudes and directions with sliders and preset buttons
- **Time Control**: Adjust simulation speed with time scale slider
- **Vertical Sync**: Toggle vertical sync for optimal performance
- **Object Deletion**: Remove objects using the delete button in the objects panel

## Technical Details

- **Rendering**: OpenGL 3.3+ with immediate mode rendering
- **Physics**: Real-time numerical integration with time step control
- **UI**: ImGui for interactive controls and parameter adjustment
- **Window Management**: GLFW for cross-platform window handling
- **Extensions**: GLEW for OpenGL extension loading
- **Physics Engine**: Custom physics implementation with cPhysics library integration
- **Collision Detection**: Basic collision resolution between objects
- **Time Management**: Delta time calculation for consistent physics across different frame rates

## Dependencies

- **GLFW**: Download link: https://www.glfw.org/download.html
- **GLEW**: Download link: https://glew.sourceforge.net/
- **ImGui**: Project link: https://github.com/ocornut/imgui
- **cPhysics**: Custom physics library for field calculations
