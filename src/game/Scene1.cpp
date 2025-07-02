#include "Scene1.h"

#include <future>

#include "Graphics/Font.hpp"
#include "Graphics/Text.hpp"

#ifdef ENABLE_IMGUI

#	include <imgui/imgui.h>
#	include <imgui/imgui_impl_opengl3.h>
#	include <imgui/imgui_impl_glfw.h>

#endif

Scene1::Scene1(
    Input& input, Physics& physics,
    Renderer& renderer, Player& player,
    const JPH::Mat44& projMatrix, JPH::Mat44& viewMatrix,
    GLFWwindow* window
)
    :   m_modelShader(
            "../resources/shaders/ModelVertex.glsl",
            "../resources/shaders/ModelFrag.glsl"
        ),
        m_input(input),
        m_physics(physics),
        m_renderer(renderer),
        m_player(player),
        m_cityModel(m_renderer, "../resources/models/city/scene.gltf", m_physics, m_frustumCuller),
        m_ar15(m_renderer, "../resources/models/ar15/scene.gltf"),
        m_spaceship1(m_renderer, "../resources/models/spaceship/scene.gltf", m_physics, m_frustumCuller),
        m_spaceship2(m_renderer, "../resources/models/spaceship2/scene.gltf", m_physics, m_frustumCuller, JPH::Mat44::sScale(2.5)),
        m_spaceship1Boss(m_spaceship1, 500),
        m_spaceship2Boss(m_spaceship2, 500),
        m_window(window),
        m_projMatrix(projMatrix),
        m_viewMatrix(viewMatrix),
        m_crosshair(-1, -1, -35, 2, 2),
        m_crosshairTexture("../resources/images/crosshair.png", Texture::TextureType::diffuse, false, false),
        m_gunFlash(1.5, -2.75, -20, 2, 2),
        m_gunFlashTexture("../resources/images/gunFlash.png", Texture::TextureType::diffuse, false, false)
{
    m_quit.store(false, std::memory_order_release);
    m_updatePhysicsNow.store(false, std::memory_order_release);

    m_frameTimings.reserve(10000);
    m_renderAndPhysicsTimings.reserve(10000);
    m_eventsSwapBuffersTimings.reserve(10000);

    m_physics.OptimizeBroadphase();

    m_spaceship1.SetRotation({0, 0, AI_MATH_HALF_PI_F});
    m_spaceship2.SetRotation({0, AI_MATH_HALF_PI_F, 0});

    //We do this twice to lock the mouse (for some reason it does not lock by default)
    m_input.FlipMouseEnabled();
    m_input.FlipMouseEnabled();

    //Initialize ImGui
#ifdef ENABLE_IMGUI
    ImGui::CreateContext();
    m_imGuiIo = &ImGui::GetIO();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);

    ImGui::StyleColorsDark();

    const char* glsl_version = "#version 430";
    ImGui_ImplOpenGL3_Init(glsl_version);
#endif
}

void Scene1::DrawDebugPhysics()
{
    m_physics.DrawDebugPhysics();
}

void Scene1::MainLoop()
{
    using namespace std::chrono;
    using clock = std::chrono::high_resolution_clock;

    bool drawDebugPhysics = false;

    clock::time_point time1;
    clock::time_point time2;
    float deltaTimeMs = 1000 / 60.f; //The deltaTime in milliseconds. We start at 16.66ms, which is the time per frame at 60FPS.

    std::thread physicsUpdateThread(&Scene1::UpdatePhysicsThread, this);
    physicsUpdateThread.detach();

    // glLineWidth(1.5);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (!glfwWindowShouldClose(m_window))
    {
        time1 = clock::now();

        m_renderer.Clear();
        ImGuiFrameStart(drawDebugPhysics);

        UpdatePlayer(deltaTimeMs);

        auto start = clock::now();
        m_deltaTime.store(deltaTimeMs, std::memory_order_release);
        m_updatePhysicsNow.store(true, std::memory_order_release); //We have called m_physics.Update() here

        // UpdatePhysics(deltaTimeMs);

        if (drawDebugPhysics)
            DrawDebugPhysics();
        else
            DrawModels();

        while (!m_doneUpdatingPhysics.load(std::memory_order_acquire)) //Wait until we are done with physics
        {
        }

        m_doneUpdatingPhysics.store(false, std::memory_order_release);
        auto end = clock::now();
        m_renderAndPhysicsTimings.emplace_back(duration_cast<microseconds>(end - start).count() / 1000.f);

        ImGuiFrameEnd(drawDebugPhysics);

        start = clock::now();
        HandleEventsAndBuffers();
        end = clock::now();
        m_eventsSwapBuffersTimings.emplace_back(duration_cast<microseconds>(end - start).count() / 1000.f);

        time2 = clock::now();
        deltaTimeMs = duration_cast<microseconds>(time2 - time1).count() / 1000.f;
        m_frameTimings.emplace_back(deltaTimeMs);

        if (m_player.GetExitProgram())
            break;
    }

    DumpStatistics();

    m_quit.store(true, std::memory_order_release);

    //The UpdatePhysicsThread functions sets it to false once it sees that it needs to exist
    while (m_quit.load(std::memory_order_relaxed))
    {}
}

void Scene1::ImGuiFrameStart(bool& drawDebugPhysics)
{
#ifdef ENABLE_IMGUI
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Control");

    if (ImGui::Button("Switch Drawing"))
        drawDebugPhysics = !drawDebugPhysics;

    if (ImGui::Button("Dump Statistics"))
        DumpStatistics();
#endif
}

void Scene1::ImGuiFrameEnd(bool &drawDebugPhysics)
{
#ifdef ENABLE_IMGUI
    ImGui::Text("drawDebugPhysics: %s", drawDebugPhysics ? "true" : "false");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / m_imGuiIo->Framerate, m_imGuiIo->Framerate);

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
}


void Scene1::UpdatePlayer(float deltaTime)
{
    m_player.Update(&m_spaceship1Boss, 2, deltaTime);
    m_viewMatrix = m_player.GetViewMatrix();
}

void Scene1::UpdatePhysicsThread()
{
    while (true)
    {
        if (m_updatePhysicsNow.load(std::memory_order_acquire))
        {
            m_updatePhysicsNow.store(false, std::memory_order_release);
            UpdatePhysics(m_deltaTime.load(std::memory_order_acquire));
            m_doneUpdatingPhysics.store(true, std::memory_order_release);

            //This will sleep for ~1ms on windows, but seems to be pretty accurate on PREEMPT_RT linux (< 100us of inaccuracy)
            std::this_thread::sleep_for(std::chrono::microseconds(500));
        }

        if (m_quit.load(std::memory_order_acquire)) [[unlikely]]
        {
            m_quit.store(false, std::memory_order_release);
            return;
        }
    }
}

void Scene1::UpdatePhysics(float deltaTimeMs)
{
    constexpr float period = AI_MATH_PI_F * 9;

    //Time will now be inbetween 0 and 9pi regardless of glfwGetTime()
    float currentTime = std::fmod(static_cast<float>(glfwGetTime()), period);
    float percentTime = currentTime / period; //This value will be between 0 and 1

    if (percentTime <= 0.25f)
    {
        if (!m_removedBoss1FromPhysics)
        {
            m_spaceship1.SetVelocity({0, 0, 0});
            m_spaceship1.SetPosition({(3 - (3 * std::sin(percentTime * (period / 4.5f)))) * 100, 20, 0});
        }

        if (!m_removedBoss2FromPhysics)
        {
            m_spaceship2.SetVelocity({0, 0, 0});
            m_spaceship2.SetPosition({(3 - (3 * std::sin(percentTime * (period / 4.5f)))) * 100, 10, 0});
        }

        goto skipProcessing;
    }
    if (percentTime <= 0.75f)
    {
        if (!m_removedBoss1FromPhysics)
        {
            m_spaceship1.SetVelocity({0, 0, 0});
            m_spaceship1.SetPosition({0, 20, 0});
        }

        if (!m_removedBoss2FromPhysics)
        {
            m_spaceship2.SetVelocity({0, 0, 0});
            m_spaceship2.SetPosition({0, 10, 0});
        }
        goto skipProcessing;
    }
    if (percentTime <= 1.f)
    {
        if (!m_removedBoss1FromPhysics)
            m_spaceship1.AddVelocity({-0.1f * deltaTimeMs, 0, 0});

        if (!m_removedBoss2FromPhysics)
            m_spaceship2.AddVelocity({-0.1f * deltaTimeMs, 0, 0});
        goto skipProcessing;
    }

    skipProcessing:
    m_physics.Update(deltaTimeMs);
}

void Scene1::DrawModels()
{
    m_modelShader.Bind();
    m_modelShader.SetUniform("u_lightPos", {0, 10, 0});

    static const JPH::Vec3 gunPos{0.18, -0.19, -0.55};
    static const JPH::Vec3 gunScale{0.0010, 0.0010, 0.0010};
    static const JPH::Vec3 gunRotation{AI_MATH_HALF_PI, 0, 0};

    static const JPH::Mat44 modelMatrixGun =
        JPH::Mat44::sTranslation(gunPos) *
        JPH::Mat44::sScale(gunScale) *
        JPH::Mat44::sRotation({1, 0, 0}, gunRotation[0]) *
        JPH::Mat44::sRotation({0, 1, 0}, gunRotation[1]) *
        JPH::Mat44::sRotation({0, 0, 1}, gunRotation[2]);

    m_modelShader.SetUniform("u_enableLighting", true);

    m_cityModel.Draw(m_modelShader, m_projMatrix, m_viewMatrix);

    if (m_spaceship1Boss.GetHealth() > 0.05)
    {
        m_spaceship1.Draw(m_modelShader, m_projMatrix, m_viewMatrix, m_spaceship1.GetModelMatrix());
    }
    else
    {
        if (!m_removedBoss1FromPhysics)
        {
            m_spaceship1.RemoveFromPhysics();
            m_removedBoss1FromPhysics = true;
        }
    }

    if (m_spaceship2Boss.GetHealth() > 0.05)
    {
        m_spaceship2.Draw(m_modelShader, m_projMatrix, m_viewMatrix, m_spaceship2.GetModelMatrix());
    }
    else
    {
        if (!m_removedBoss2FromPhysics)
        {
            m_spaceship2.RemoveFromPhysics();
            m_removedBoss2FromPhysics = true;
        }
    }

    glClear(GL_DEPTH_BUFFER_BIT);

    m_ar15.Draw(m_modelShader, m_projMatrix, JPH::Mat44::sIdentity(), modelMatrixGun);

    glClear(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    m_modelShader.SetUniform("u_enableLighting", false);

    m_crosshairTexture.Bind(m_crosshairTextureSlot);
    m_modelShader.SetUniform("u_textureDiffuse0", m_crosshairTextureSlot);
    m_crosshair.Draw(m_modelShader, m_renderer, m_projMatrix);

    if (m_input.IsKeyCurrentlyPressed(GLFW_MOUSE_BUTTON_LEFT) && std::rand() % 2 == 0)
    {
        m_gunFlashTexture.Bind(m_gunFlashTextureSlot);
        m_modelShader.SetUniform("u_textureDiffuse0", m_gunFlashTextureSlot);
        m_gunFlash.Draw(m_modelShader, m_renderer, m_projMatrix);
    }

    m_spaceship1Boss.DrawHealthBar(m_modelShader, m_renderer, m_projMatrix, "u_textureDiffuse0");
    m_spaceship2Boss.DrawHealthBar(m_modelShader, m_renderer, m_projMatrix, "u_textureDiffuse0");

    glEnable(GL_DEPTH_TEST);
}

void Scene1::HandleEventsAndBuffers()
{
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

void Scene1::DumpStatistics()
{
    auto average = [](const std::vector<double>& v) -> double {
        if (v.empty()) return 0.0;
        return std::accumulate(v.begin(), v.end(), 0.0) / v.size();
    };

    auto maximum = [](const std::vector<double>& v) -> double {
        if (v.empty()) return 0.0;
        return *std::max_element(v.begin(), v.end());
    };

    auto standardDeviation = [](const std::vector<double>& v) -> double {
        if (v.size() < 2) return 0.0;
        double avg = std::accumulate(v.begin(), v.end(), 0.0) / v.size();
        double sumSqDiff = std::accumulate(v.begin(), v.end(), 0.0,
            [avg](double acc, double x) {
                return acc + (x - avg) * (x - avg);
            });
        return std::sqrt(sumSqDiff / (v.size() - 1));
    };

    auto printStats = [&](const std::string& label, const std::vector<double>& data) {
        std::cout << label << ":\n";
        std::cout << "  Average Time: " << average(data) << " ms\n";
        std::cout << "  Max Time:     " << maximum(data) << " ms\n";
        std::cout << "  Std Dev:      " << standardDeviation(data) << " ms\n";
    };

    printStats("Overall frame timings", m_frameTimings);
    printStats("Physics + Render frame timings", m_renderAndPhysicsTimings);
    printStats("Events + Swap buffer frame timings", m_eventsSwapBuffersTimings);

    m_frameTimings.clear();
    m_renderAndPhysicsTimings.clear();
    m_eventsSwapBuffersTimings.clear();
}
