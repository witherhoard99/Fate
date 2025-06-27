#include "Application.h"

int main()
{
    Application::InitError error;
    GLFWwindow* window = Application::InitLibraries(error);

    ASSERT_LOG(
        error == Application::InitError::NoError,
        "Failed to initialize application. Error code: InitError::" << static_cast<uint8>(error)
    );

    //Because of the way OpenGL, our rendering API, works we need this scope here so that the destructor for Application
    //can be called before we call Application::ShutdownLibraries()
    {
        Application app{window};
        app.Run();
    }

    Application::ShutdownLibraries();
}
