# TODO:
  * Do the other TODOs in the README (make the binaries, and link their URL in the README)
  * Fix FrustumCullling.h
    * The bodies were never added to the JPH::PhysicsSystem, so the narrow phase query does not work
    * Either use the JPH::PhysicsSystem we already have, or add all of the bodies to the new physicsSystem


# Fate
Fate is a 3D video game made in C++ using OpenGL and GLSL. 

# How to Play
* Windows and Linux binary is inside releases: (TODO: INSERT URL HERE)
* Uses WASD for movement
  * W is forward
  * S is backward
  * A is right
  * D is left
* Use mouse for looking around
* Left click is shoot
* Press shift to move faster



# Compilation

### Platforms
* Linux with GCC + ninja: Tested extensively 
* Windows with MinGW: Assimp requires source code to be modified, works otherwise
  * TODO: CHECK THIS
* Windows with Visual Studio: Not tested, but should work


### Notes
* Mainly compiled/tested on Linux with GCC and Ninja
* Assimp is very difficult on compile on MinGW, but all other code should compile. 
  * Incase you are unable to compile Assimp, Windows and Linux binaries for Assimp are also in releases: (TODO: INSERT URL HERE)
* While not tested on Visual Studio and MSVC, it should work
* The windows binary was cross compiled

### Commands
* TODO: CHECK THESE COMMANDS ARE CORRECT ON LINUX AND WINDOWS
* Replace `[num threads]` with however number of threads you have
```
    git clone https://github.com/witherhoard99/Fate.git
    cd Fate
    cmake -S . -B ./build/release
    cmake --build -j [num threads]
```

# Libraries

### Main Dependencies

* Jolt Physics - Multithreaded 3D physics engine
* GLEW      - Loads OpenGL function pointers, necessary to be able to call any OpenGL function
* GLFW      - Platform agnostic windowing and input library.
* Assimp    - Used to load models from any file type into one consistent interface and binary format 
* SFML      - Includes many things for 2D games, however, Fate uses it for Audio
* GLM       - Mathmatics Library for games (Matrix multiplication, inverse, etc)
* STB Image - Used to load most image types into a binary format

### Dev Dependency
* ImGui - Useful for tuning / modifying values at runtime, saves lots of time
  * Is not required if DISABLE_IMGUI is set
