cmake_minimum_required(VERSION 3.25)
project(learnOpenGL)

# General sets
set(CMAKE_CXX_STANDARD 23)

# Assimp sets
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE INTERNAL "" FORCE)

# Jolt set
set(CPP_RTTI_ENABLED ON CACHE INTERNAL "" FORCE)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(OpenGL_GL_PREFERENCE GLVND)
endif()

# All compile definitions
# assimp has "#pragma warnings" that stop compilation due to -Werror
add_compile_options(-Wno-unknown-pragmas)
add_compile_definitions(GLEW_STATIC)

# Debug, Release, Distribution, and Release with debug info are supported
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug mode: Adding -g3 -O0 -ggdb")
    set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -ggdb")
    add_compile_definitions(_DEBUG)
    add_compile_definitions(JPH_DEBUG)
    add_compile_definitions(ENABLE_IMGUI)
    add_compile_definitions(JPH_DEBUG_RENDERER)
#    add_compile_definitions(JPH_PROFILE_ENABLED)
elseif ((CMAKE_BUILD_TYPE STREQUAL "Release") OR (CMAKE_BUILD_TYPE STREQUAL "Distribution"))
    message(STATUS "Release mode: Adding -O3")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    add_compile_definitions(NDEBUG)
elseif (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    add_compile_definitions(NDEBUG)
    add_compile_definitions(_DEBUG)
    add_compile_definitions(JPH_DEBUG)
    add_compile_definitions(ENABLE_IMGUI)
    add_compile_definitions(JPH_DEBUG_RENDERER)
#    add_compile_definitions(JPH_PROFILE_ENABLED)
else ()
    message(WARNING "Unknown build type. ${CMAKE_BUILD_TYPE}")
endif()

# Add all files
add_executable(learnOpenGL
        src/Util.h
        src/WindowsOnly.h

        src/VertexBuffer.cpp
        src/VertexBuffer.h
        src/IndexBuffer.cpp
        src/IndexBuffer.h
        src/VertexArray.cpp
        src/VertexArray.h
        src/VertexBufferLayout.cpp
        src/VertexBufferLayout.h
        src/Shader.cpp
        src/Shader.h
        src/Renderer.cpp
        src/Renderer.h
        src/Texture.cpp
        src/Texture.h

        src/Quads3D.cpp
        src/Quads3D.h
        src/Constants.h
        src/Player.cpp
        src/Player.h
        src/Input.cpp
        src/Input.h

        src/Physics.cpp
        src/Physics.h
        src/PhysicsObjectFactory.cpp
        src/PhysicsObjectFactory.h

        src/Model.cpp
        src/Model.h
        src/StaticModel.cpp
        src/StaticModel.h
        src/DynamicModel.cpp
        src/DynamicModel.h
        src/Mesh.cpp
        src/Mesh.h

        src/FrustumCulling.h

        src/game/truemain.cpp
        src/game/Application.cpp
        src/game/Application.h
        src/game/Scene1.cpp
        src/game/Scene1.h
        src/game/Boss.cpp
        src/game/Boss.h

        vendor/imgui/imconfig.h
        vendor/imgui/imgui.cpp
        vendor/imgui/imgui.h
        vendor/imgui/imgui_demo.cpp
        vendor/imgui/imgui_draw.cpp
        vendor/imgui/imgui_impl_glfw.cpp
        vendor/imgui/imgui_impl_glfw.h
        vendor/imgui/imgui_impl_opengl3.cpp
        vendor/imgui/imgui_impl_opengl3.h
        vendor/imgui/imgui_impl_opengl3_loader.h
        vendor/imgui/imgui_internal.h
        vendor/imgui/imgui_tables.cpp
        vendor/imgui/imgui_widgets.cpp
        vendor/imgui/imstb_rectpack.h
        vendor/imgui/imstb_textedit.h
        vendor/imgui/imstb_truetype.h

        vendor/stb_image/stb_image.cpp
        vendor/stb_image/stb_image.h
        src/Audio.cpp
        src/Audio.h
        src/Quads2D.cpp
        src/Quads2D.h
)

# More assimp stuff (https://github.com/unsettledgames/debut-engine/blob/main/Debut/CMakeLists.txt)
# Also, for some reason this configure_file seems to be super inconsistent on linux
# It will say file not found half the time but not the other half of the time
configure_file("${CMAKE_SOURCE_DIR}/Dependencies/assimp/include/assimp/config.h.in" "${CMAKE_SOURCE_DIR}/Dependencies/assimp/include/assimp/config.h")
configure_file("${CMAKE_SOURCE_DIR}/Dependencies/assimp/contrib/zlib/zconf.h.cmakein" "${CMAKE_SOURCE_DIR}/Dependencies/assimp/contrib/zlib/zconf.h")

# All add subdirectories
add_subdirectory(${CMAKE_SOURCE_DIR}/Dependencies/Jolt/Build/)
add_subdirectory(${CMAKE_SOURCE_DIR}/Dependencies/glfw/)
add_subdirectory(${CMAKE_SOURCE_DIR}/Dependencies/glew-2.1.0/build/cmake)
add_subdirectory(${CMAKE_SOURCE_DIR}/Dependencies/assimp/)

add_subdirectory(${CMAKE_SOURCE_DIR}/Dependencies/sfml/)

# All including directories
# This is so that we an include without doing "../" or "game/" within our own src files
target_include_directories(learnOpenGL PRIVATE ${CMAKE_SOURCE_DIR}/src/game)
target_include_directories(learnOpenGL PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_include_directories(learnOpenGL PRIVATE ${CMAKE_SOURCE_DIR}/Dependencies/assimp/include)
target_include_directories(learnOpenGL PRIVATE ${CMAKE_SOURCE_DIR}/Dependencies/glew-2.1.0/include)
target_include_directories(learnOpenGL PRIVATE ${CMAKE_SOURCE_DIR}/vendor/)
target_include_directories(learnOpenGL PRIVATE ${CMAKE_SOURCE_DIR}/Dependencies/sfml/include/SFML)
target_include_directories(learnOpenGL PRIVATE
        "${CMAKE_SOURCE_DIR}/Dependencies/glfw/"
        "${CMAKE_SOURCE_DIR}/Dependencies/glfw/include/GLFW/"
)
target_include_directories(learnOpenGL PRIVATE
        "${CMAKE_SOURCE_DIR}/Dependencies/Jolt/Jolt"
        "${CMAKE_SOURCE_DIR}/Dependencies/Jolt/Build"
)

if (CMAKE_CROSSCOMPILING)
    message(STATUS "Cross compiling from linux to windows.")
endif ()

if (WIN32)
    target_link_libraries(learnOpenGL winmm) # We need this for the timerBeginPeriod and timerEndPeriod calls

    # I'm not sure from what library, but we have massive stack allocations that can sometimes cause a stack overflow on windows
    # So we need to increase the stack size
    # My guess is that it is from Jolt (JPH_PROFILE_THREAD)?
    # Linux allocates more stack space by default so we are fine on linux, and linux stack size is set at runtime anyway
    if (MSVC)
        target_link_options(learnOpenGL PRIVATE "/STACK:8388608")
    else ()
        target_link_options(learnOpenGL PRIVATE "-Wl,--stack=8388608") # Should be clang or gnu
    endif()
endif ()

# All linking of libraries
target_link_libraries(learnOpenGL glfw)
target_link_libraries(learnOpenGL Jolt)
target_link_libraries(learnOpenGL glew_s)
target_link_libraries(learnOpenGL assimp)
target_link_libraries(learnOpenGL SFML::Audio SFML::Graphics)

# OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(learnOpenGL OpenGL::GL)

