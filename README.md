## About
ThING is a custom graphics engine built on top of the **Vulkan api**, designed for real-time data structure and algorithm visualization.
### Rendering
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


## Design
ThING was built specially for:
- **Graph visualization**
- **Algorithm animation**
- **Data structure simulation**
- **Real-time interaction**

Because of this:
- Polygon rendering is not that polished
- API design is data oriented and permissive
- render is optimized for large amount of primitives


## Usage
Interaction with the engine is done through the ThING::API class.
The engine provides two main entry points (both optional):

- `void update(ThING::API&, FPSCounter)`
- `void ui(ThING::API&, FPSCounter)`

This allows separating core logic from UI code.
To use the engine in your own project:
```
add_subdirectory(ThING-Engine)
add_executable(test main.cpp)
target_link_libraries(test PRIVATE ThING)
```
There's an integration example in [Emerald_Green](https://github.com/Zellterics/EmeraldGreen)

## Tests
Performance tests were run using a **RTX** 4060**TI**.
![Demo Example](/demo/demo_example.png)

The **demo/** folder contains a demonstration project that includes:
- Circle physics simulation
- Straight line rendering test
- Polygon generator
- Interactive UI for configuration
- Left click on window generates a circle
- right click on window erases circle

## Notes
- Different size outlines are supported but with irregularities between them due to limitations with the JFA algorithm.

