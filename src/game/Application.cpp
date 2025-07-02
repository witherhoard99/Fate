//Engine Includes
#include "Application.h"
#include "Scene1.h"
#include "WindowsOnly.h"

//Jolt includes
#include <Jolt/Jolt.h>
#include <Jolt/Core/Core.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>

//Stl includes
#include <cstdarg>

//Anonymous namespace for most callbacks (so that they cannot be accessed outside this file)
namespace
{
    void GLFWErrorCallback(int errorCode, const char* description)
    {
        ASSERT_LOG(false, "GLFW error!\n\tError code: " << errorCode << "\n\tDescription: " << description);
    }

    void OpenGLDebugCallback(
        GLenum source, GLenum type,
        unsigned int id,  GLenum severity,
        GLsizei length, const char *message,
        const void *userParam
    )
    {
        if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
        {
            return; //We only want actual errors or warnings
        }

        std::cerr << "\n---------------OPENGL ERROR---------------\n" << std::endl;

        switch (severity)
        {
            case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
            case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
            case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
        }
        std::cerr << std::endl;

        switch (source)
        {
            case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
            case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
            case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
            case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
            case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
        }
        std::cerr << std::endl;

        switch (type)
        {
            case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
            case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
            case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
            case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
            case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
            case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
            case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
        }
        std::cerr << std::endl;

        std::cerr << message << std::endl;
        ASSERT_LOG(false, "OpenGL error!");
    }

    void JoltTraceImpl(const char *inFMT, ...)
    {
        va_list list;
        va_start(list, inFMT);
        char buffer[2048];
        vsnprintf(buffer, sizeof(buffer), inFMT, list);
        va_end(list);

        std::cerr << buffer << std::endl;
        JPH_ASSERT(false);
    }

    bool JoltAssertFailedImpl(const char *inExpression, const char *inMessage, const char *inFile, uint inLine)
    {
        std::cerr << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr? inMessage : "") << std::endl;
        return true;
    }
}

Application::Application(GLFWwindow* window)
    :   m_physicsShader("../resources/shaders/PhysicsVertex.glsl", "../resources/shaders/PhysicsFrag.glsl"),
        m_window(window),
        m_projMatrix(JPH::Mat44::sPerspective(
            JPH::DegreesToRadians(75.f),
            (float)Util::options.scrWidth / (float)Util::options.scrHeight,
            0.1f,
            1000.f
        )),
        m_input(m_window),
        m_player(m_input, nullptr),
        m_viewMatrix(m_player.GetViewMatrix()),
        m_physics(m_physicsShader, m_renderer, m_projMatrix, m_viewMatrix, m_player.GetPosition())
{
    m_player.SetCharacterHandler(m_physics.GetCharacterHandler());
}

Application::~Application()
{
    if (m_window != nullptr)
        glfwDestroyWindow(m_window);
}

GLFWwindow* Application::InitLibraries(InitError &error)
{
    EnableHighResolutionSleeps();

    GLFWwindow* window;
    error = InitGLFW(window);

    if (error != InitError::NoError)
        return nullptr;

    auto glewInitCode = glewInit();
    if (glewInitCode != GLEW_OK && glewInitCode != 4) //The 4 is because this can be emitted on linux wayland, and does not mean anything
    {
        error = InitError::UnableToInitGLEW;

        glfwDestroyWindow(window);
        glfwTerminate();
        return nullptr;
    }

    InitOpenGL();
    InitJolt();

    return window;
}

void Application::ShutdownLibraries()
{
    DisableHighResolutionSleeps();
    glfwTerminate();
}

int Application::GetScene()
{
    //TODO: Read from a file here to get the current scene
    return 1;
}

void Application::Run()
{
    //This is probably the most efficient way of getting and running scenes
    //This is because the memory for each scene will be allocated and destroyed on the stack
    //otherwise we would need to heap allocate which would reduce performance
    //There is currently only one scene, but we don't want to limit our app to just 1 scene
    int sceneNum = GetScene();
    if (sceneNum == 1)
    {
        Scene1 scene{
            m_input, m_physics,
            m_renderer, m_player,
            m_projMatrix, m_viewMatrix,
            m_window
        };

        scene.MainLoop();
        sceneNum++;
    }
}


Application::InitError Application::InitGLFW(GLFWwindow *&window)
{
    JPH_IF_DEBUG(glfwSetErrorCallback(GLFWErrorCallback));

    if (!glfwInit())
    {
        return InitError::UnableToInitGLFW;
    }

    const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    Util::options.scrWidth = mode->width;
    Util::options.scrHeight = mode->height;

    JPH_IF_DEBUG(glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE));
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, Util::Options::openGLVersionMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, Util::Options::openGLVersionMinor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if (Util::options.numMSAASamples > 1)
        glfwWindowHint(GLFW_SAMPLES, Util::options.numMSAASamples);

    window = glfwCreateWindow(
        Util::options.scrWidth,
        Util::options.scrHeight,
        Util::options.windowTitle,
        Util::options.fullscreen ? glfwGetPrimaryMonitor() : nullptr,
        nullptr
    );

    if (!window)
    {
        glfwTerminate();
        return InitError::UnableToMakeWindow;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(Util::options.vsync ? 1 : 0);

    return InitError::NoError;
}


void Application::InitOpenGL()
{
    glViewport(0, 0, Util::options.scrWidth, Util::options.scrHeight);

    JPH_IF_DEBUG(
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(OpenGLDebugCallback, nullptr);
    );

    if (Util::options.numMSAASamples > 1)
        glEnable(GL_MULTISAMPLE);

    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);
    // glFrontFace(GL_CCW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.5f, 0.09f, 0.06f, 1.0f); //TODO: Replace with a cubemap
}



void Application::InitJolt()
{
    JPH::RegisterDefaultAllocator();
    JPH::Trace = JoltTraceImpl;
    JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = JoltAssertFailedImpl;)

    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();

#ifdef JPH_PROFILE_ENABLED
    JPH::Profiler::sInstance = new JPH::Profiler();
#endif
}


