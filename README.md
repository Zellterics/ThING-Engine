## About
ThING is a Vulkan-based engine designed specifically for large-scale algorithm and data structure visualization.

It is built to render and interact with hundreds of thousands to millions of primitives in real time.

## Design
ThING was built specially for:
- **Graph visualization**
- **Algorithm animation**
- **Data structure simulation**
- **Real-time interaction**

Because of this:
- Polygon rendering is not that polished
- API design is data oriented and permissive
- Render is optimized for large amount of primitives

## Quick Start
```c++
#include <ThING/api.h>

void updateCallback(ThING::API& api, FPSCounter fps){
    api.addCircle({1,1}, 30, {1,0,1,1});
}

int main(){
    ThING::API api;
    api.setUpdateCallback(updateCallback);
    api.run();
}
```

## Rendering
- **Circles** — up to **~1M (2²⁰)** instances  
    Performance depends on **average circle size (screen coverage)**.
    
- **Lines** — up to **~1M** instances  
    Performance depends on **line length and thickness in screen space**.
    
- **Polygons** — up to **~200k** instances  
    Performance depends on **vertex complexity and overlap**.
    
- **Outlines** — **Jump Flood Algorithm (JFA)**  
    Full-screen multi-pass; cost depends on **render resolution**, not object count.

### UI & Audio
- ImGui for interfaces
- miniaudio for audio playback

## Usage
Interaction with the engine is done through the ThING::API class.
The engine provides two optional callbacks:

- `void update(ThING::API&, FPSCounter)`
- `void ui(ThING::API&, FPSCounter)`

FPSCounter provides frame timing information for profiling and debugging.

This allows separating core logic from UI code.

To use the engine in your own project add the subdirectory and libraries to your CMake file:
```
add_subdirectory(ThING-Engine)
add_executable(test main.cpp)
target_link_libraries(test PRIVATE ThING)
```
An integration example can be found in [Emerald_Green](https://github.com/Zellterics/EmeraldGreen)

# Setup

The engine itself does not provide a `main()` entry point.

The `demo/` folder contains a standalone application used as a testbed
to validate engine features and measure performance.

## 1. Requirements
- C++20 compatible compiler
- Tested with GCC and MSVC.

## 2. Install Vulkan
### Windows
Install the Vulkan SDK from the official LunarG website.

### Linux
#### Arch
Run `sudo pacman -S vulkan-icd-loader vulkan-tools vulkan-validation-layers` to install the Vulkan tools and validation layers.

#### Other
Install the Vulkan loader, tools and validation layers using your distribution's package manager.

### 3. Clone Repository
This project uses git submodules for ImGui, GLM, GLFW.
```bash
git clone --recurse-submodules https://github.com/you/ThING.git
```
## Demo / Test Application
### Building the demo

The demo application is built using the CMake project located in `./demo`.

The repository includes a preconfigured `.vscode` folder for building and running the demo using VSCode.

#### VSCode
Open the repository root and use the CMake extension to configure and build the project.

#### Visual Studio
Open the `./demo` directory as a CMake project and build the demo target.

![Demo Example](/demo/demo_example.png)

The **demo/** folder contains a demonstration project that includes:
- Circle physics simulation
- Straight line rendering test
- Polygon generator
- Interactive UI for configuration
- Left click on window generates a circle
- right click on window erases circle

## Notes
- The demo application is intended only as a development and benchmarking tool.
- Performance tests were run using a **RTX 4060 Ti**.
- Different size outlines are supported but with irregularities between them due to limitations with the JFA algorithm.
