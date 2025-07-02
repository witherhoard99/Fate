# About Fate
Fate is a 3D video game made in C++ using OpenGL and GLSL. It does not make use of any game engines.

<br/>

# How to

### How to Play
* Uses WASD for movement
  * W is forward
  * S is backward
  * A is right
  * D is left
* Use mouse for looking around
* Left click is shoot
* Press shift to move faster

### How to Run
* Binaries are located [here](https://github.com/witherhoard99/Fate/releases/tag/v0.1.0)
* There are Debug, Release, ReleaseWithDebugInfo binaries for both Linux and Windows

<br/>

# Compilation

### Platforms
* Linux with GCC + ninja: Tested extensively 
* Windows with MinGW: Assimp requires source code to be modified, works otherwise
  * TODO: CHECK THIS
* Windows with Visual Studio: Not tested, but should work


### Notes
* Mainly compiled/tested on Linux with GCC and Ninja
* Assimp is very difficult to compile on MinGW, but all other code should compile. 
* While not tested on Visual Studio and MSVC, it should work
* The windows binaries were cross compiled

### Commands
* TODO: CHECK THESE COMMANDS ARE CORRECT ON LINUX AND WINDOWS
* Replace `[num threads]` with the number of threads you want (or use `$(nproc)` on linx)
```
    git clone https://github.com/witherhoard99/Fate.git
    cd Fate
    cmake -S . -B ./build/release -DCMAKE_BUILD_TYPE="Release" -DCMAKE_POLICY_VERSION_MINIMUM="3.5"
    cmake --build ./build/release -j [num threads]
```

<br/>

# Libraries

### Main Dependencies

<pre>
* Jolt         -   Multithreaded 3D physics engine
* GLEW         -   Loads OpenGL function pointers, necessary to be able to call any OpenGL function
* GLFW         -   Platform agnostic windowing and input library
* Assimp       -   Used to load models of any file type into one consistent interface and binary format 
* SFML         -   Includes many things for 2D games, however, Fate uses it for Audio
* STB Image    -   Used to load most image types into a binary format
</pre>

### Dev Dependency
<pre>
* ImGui - Useful for tuning / modifying values at runtime, saves lots of time
  * Is not required if DISABLE_IMGUI is set
</pre>

<br/>

# Other

### Notes
Some of the commit messages are inaccurate because I did not realize I have to do `git add --all` every single commit. 
Therefore, some commit messages apply to later commits 

### TODO:
* Do the other TODOs in the README (make the binaries, and link their URL in the README)
* Fix FrustumCullling.h's shape testing (AABB works well)
    * The bodies were never added to the JPH::PhysicsSystem, so the narrow phase query does not work
    * Either use the JPH::PhysicsSystem we already have, or add all of the bodies to the new physicsSystem
* Add more advanced lighting, with support for multiple lights
