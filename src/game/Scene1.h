#ifndef SCENE1_H
#define SCENE1_H

#include "Boss.h"
#include "DynamicModel.h"
#include "Util.h"
#include "Input.h"
#include "Physics.h"
#include "Player.h"
#include "Shader.h"
#include "Model.h"
#include "Quads2D.h"
#include "Quads3D.h"
#include "StaticModel.h"
#include "imgui/imgui.h"

class Scene1
{
private:
    Shader          m_modelShader;
    FrustumCuller   m_frustumCuller;

    Input&          m_input;
    Physics&        m_physics;
    Renderer&       m_renderer;
    Player&         m_player;

    StaticModel     m_cityModel;
    Model           m_ar15;
    DynamicModel    m_spaceship1;
    DynamicModel    m_spaceship2;

    Boss            m_spaceship1Boss;
    Boss            m_spaceship2Boss;

    GLFWwindow*     m_window;

    ImGuiIO*        m_imGuiIo;

    const JPH::Mat44&   m_projMatrix;
    JPH::Mat44&         m_viewMatrix;

    Quads2D                 m_crosshair;
    Texture                 m_crosshairTexture;
    static constexpr uint   m_crosshairTextureSlot = 0;

    Quads2D                 m_gunFlash;
    Texture                 m_gunFlashTexture;
    static constexpr uint   m_gunFlashTextureSlot = 0;

    //Only to track statistics on how long everything is taking
    std::vector<double> m_frameTimings;
    std::vector<double> m_renderAndPhysicsTimings;
    std::vector<double> m_eventsSwapBuffersTimings;

    bool m_removedBoss1FromPhysics = false;
    bool m_removedBoss2FromPhysics = false;

    //These are all for the UpdatePhysicsThread() function
    std::atomic<float>  m_deltaTime;
    std::atomic<bool>   m_updatePhysicsNow;
    std::atomic<bool>   m_doneUpdatingPhysics;
    std::atomic<bool>   m_quit;

    void DrawDebugPhysics();

    void ImGuiFrameStart(bool& drawDebugPhysics);
    void ImGuiFrameEnd(bool& drawDebugPhysics);

    void UpdatePlayer(float deltaTime);

    void UpdatePhysicsThread();
    void UpdatePhysics(float deltaTimeMs);

    void DrawModels();

    void HandleEventsAndBuffers();

    void DumpStatistics();

public:

    Scene1(
        Input& input, Physics& physics,
        Renderer& renderer, Player& player,
        const JPH::Mat44& projMatrix, JPH::Mat44& viewMatrix,
        GLFWwindow* window
    );

    void MainLoop();
};



#endif //SCENE1_H
