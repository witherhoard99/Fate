# About Fate
Fate is a 3D video game and game engine made in C++ using OpenGL and GLSL. 

<br/>

# How to

### How to Play
* Uses WASD for movement
  * W to go forward
  * S to go backward
  * A to go right
  * D to go left
  * Space to go up
* Use mouse for looking around
* Left click to shoot
* Press shift to move faster

### How to Run
* Binaries are located [here](https://github.com/witherhoard99/Fate/releases/tag/v0.1.0)
* There are Debug, Release, ReleaseWithDebugInfo binaries for both Linux and Windows

<br/>

# Compilation

### Platforms
* Linux with GCC + ninja: Tested extensively 
* Windows with MinGW: Assimp requires source code to be modified (use the assimp library in Dependencies), works otherwise
* Windows with Visual Studio: Not tested


### Notes
* Mainly compiled/tested on Linux with GCC and Ninja
* Assimp is very difficult to compile on MinGW, but all other code should compile. 
* While not tested on Visual Studio and MSVC, it should work
* The windows binaries were cross compiled

### Commands
* Replace `[num threads]` with the number of threads you want (or use `$(nproc)` on linx)
* If you are on Windows, replace the forward slashes with backslashes (`./cmake-build-release/` -> `.\cmake-build-release\`)
```
    git clone https://github.com/witherhoard99/Fate.git
    cd Fate
    cmake -S . -B ./cmake-build-release/ -DCMAKE_BUILD_TYPE="Release" -DCMAKE_POLICY_VERSION_MINIMUM="3.5"
    cmake --build ./cmake-build-release/ -j [num threads]
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

### Future Enhancements:
* Fix FrustumCullling.h's shape testing (AABB works well)
    * The bodies were never added to the JPH::PhysicsSystem, so the narrow phase query does not work
    * Either use the JPH::PhysicsSystem we already have, or add all of the bodies to the new physicsSystem
* Add more advanced lighting, with support for multiple lights
