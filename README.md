# MarbleEngine

Our terrain generation and 3D rendering engine for our software quality course.

![Screenshot at the begining of the project](https://user-images.githubusercontent.com/44240178/193861277-f4d03973-bde5-438e-83f4-a35e247530ef.png)

## Final screenshots

![final_1](_git/final_1.png)
![final_2](_git/final_2.png)
![final_3](_git/final_3.png)
![final_4](_git/final_4.png)
![final_5](_git/final_5.png)
![final_6](_git/final_6.png)
![final_7](_git/final_7.png)

### Building

This project was built using Visual Studio, building with other platforms should not be too different.

Steps for VS:
- clone or download the project
- create a new VS project in the new directory
- go to the project's properties > C/C++ > additional include directories > add `$(ProjectDir)Libraries\include`
- in the Linker tab > general > additional library directories > add `$(ProjectDir)Libraries\lib`
- in the Linker tab > input > additional dependencies > add `glfw3.lib`
- in the General tab, make sure you are using C++20 or latter

### roadmap

**OpenGL abstraction**:
- [x] VBO/IBO/Shaders
- [x] Mesh
- [x] FBO
- [x] On-the-fly shader recompilation

**Terrain**:
- [x] Heightmap generation
- [x] Mesh generation
- [x] Realistic generation (=erosion)

**Monde**:
- [x] Features (trees/grass...)
- [x] Skybox
- [x] Clouds
- [ ] ? Day/night cycle
- [x] Rivers/Oceans

**Camera**:
- [x] Player movements
- [x] Perspective/Isometric

**VFX**:
- [x] Ambiant shadows
- [x] Casted shadows
- [ ] ? Ambiant occlusion
- [x] HDR
- [x] Bloom
- [x] Vignette
- [x] Distance fog
- [x] Dirt mask

**Animations**:
- [ ] Mesh animations
- [x] ? Wind
- [ ] ? Rain
- [x] Nuages
- [ ] ? Day/night cycle

