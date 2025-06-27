#ifndef APPLICATION_H
#define APPLICATION_H

#include "../src/Input.h"
#include "../src/Player.h"
#include "../src/Util.h"

class Application
{
public:
    enum class InitError : uint8
    {
        NoError = 0,
        UnableToMakeWindow,
        UnableToInitGLFW,
        UnableToInitGLEW
    };

private:
    static InitError    InitGLFW(GLFWwindow *&window);
    static void         InitOpenGL();
    static void         InitJolt();

    //This is to be called when initializing goes wrong and we need to exit
    void EarlyTerminate();

    int GetScene();

public:

    Shader      m_physicsShader;
    GLFWwindow* m_window;

    JPH::Mat44  m_projMatrix;

    Input       m_input;
    Player      m_player;

    JPH::Mat44  m_viewMatrix;

    Physics     m_physics;
    Renderer    m_renderer;

    //Calls all of the other private init functions
    explicit Application(GLFWwindow* window);
    ~Application();

    //These 2 function exist because some things must be done before or after EVERYTHING else
    //Within the constructor, we need init our class members, but our libraries need to init'd before that
    static GLFWwindow*  InitLibraries(InitError& initError);

    //Likewise, glfwTerminate() needs to called after ALL other destructors (including class members) are called
    static void         ShutdownLibraries();

    void Run();
};

#endif //APPLICATION_H
