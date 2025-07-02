#include "Physics.h"
#include <unordered_set>

#include "Constants.h"

Physics::Physics(Shader &shader, Renderer &renderer, const JPH::Mat44& projMatrix, const JPH::Mat44& viewMatrix, const JPH::Vec3 &cameraPosition) :
    m_tempAllocator(10 * 1024 * 1024),
    m_shader(shader),
    m_renderer(renderer),
    m_projMatrix(projMatrix),
    m_viewMatrix(viewMatrix),
    /*m_jobSystem(
        1024,
        std::clamp((int)std::thread::hardware_concurrency() / 2, 12, 12) + 2,
        std::clamp((int)std::thread::hardware_concurrency() / 2, 12, 12)
    ),*/
    m_jobSystem(std::pow(2, 20))
    JPH_IF_DEBUG_RENDERER(,m_debugRenderer(shader, cameraPosition))
#ifdef  JPH_PROFILE_ENABLED
    ,m_profiler(JPH::Profiler::sInstance),
    m_profileThread("JPH Main Thread")
#endif
{
    JPH_IF_DEBUG_RENDERER(
        m_drawSettings.mDrawMassAndInertia = true;
        m_drawSettings.mDrawShapeColor = JPH::BodyManager::EShapeColor::InstanceColor;
    )

    const uint cMaxBodies = 2048;
    const uint cNumBodyMutexes = 0; //0 is default settings
    const uint cMaxBodyPairs = 4096;
    const uint cMaxContactConstraints = 8192;

    m_physicsSystem.Init(
        cMaxBodies, cNumBodyMutexes, cMaxBodyPairs,
        cMaxContactConstraints, m_BPLayerInterface, m_objVsBPLayerFilter,
        m_objLayerPairCollisonFilter
    );
    m_bodyInterface = &m_physicsSystem.GetBodyInterface();

    //Register BodyActivationListeners or ContactListener here if needed
    //Register CollisionCallbacks here if needed

    m_characterHandler.Init(m_physicsSystem);

#ifdef JPH_PROFILE_ENABLED
    m_profiler->AddThread(&m_profileThread);
#endif
}


Physics::~Physics()
{
#ifdef JPH_PROFILE_ENABLED
    m_profiler->Dump("Last Frame");
    m_profiler->NextFrame();
    m_profiler->RemoveThread(&m_profileThread);
#endif

    m_characterHandler.Destruct();
    if (m_bodyIDs.size() > 0)
    {
        m_bodyInterface->RemoveBodies(&m_bodyIDs[0], m_bodyIDs.size());
        m_bodyInterface->DestroyBodies(&m_bodyIDs[0], m_bodyIDs.size());
    }

    JPH::UnregisterTypes();

    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;
}


void Physics::Update(float deltaTime)
{
#ifdef JPH_PROFILE_ENABLED
    m_profiler->NextFrame();
#endif

    constexpr float frameTime60FPS = 1000 / 60.f;

    int collisonSteps = 1;
    float ratioDeltaTimeFrameTime = deltaTime / frameTime60FPS;

    if (ratioDeltaTimeFrameTime >= 1.05)
        collisonSteps = std::min(std::ceil(ratioDeltaTimeFrameTime), 100.f);

    auto start = std::chrono::high_resolution_clock::now();

    JPH::EPhysicsUpdateError error = m_physicsSystem.Update(deltaTime / 1000.f, collisonSteps, &m_tempAllocator, &m_jobSystem);
    ASSERT_LOG(error == JPH::EPhysicsUpdateError::None, "JPH Physics Update Error: " << static_cast<uint32>(error));

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000;
    if (duration > 32)
    {
#ifdef JPH_PROFILE_ENABLED
        m_profiler->Dump();
#endif
        std::cout << "Physics::Update took long time: " << duration << "ms\tDelta time: " << deltaTime << "ms\tratioDeltaTimeFrame: " <<
            ratioDeltaTimeFrameTime << "\tcollisonSteps: " << collisonSteps << std::endl;
    }

    m_characterHandler.UpdateCharacter();
}

void Physics::DrawDebugPhysics()
{
#ifdef JPH_DEBUG_RENDERER
    m_shader.Bind();
    m_shader.SetUniform("u_MVP", m_projMatrix * m_viewMatrix);

    m_debugRenderer.StartFrame();
    m_physicsSystem.DrawBodies(m_drawSettings, &m_debugRenderer);
    m_debugRenderer.EndFrame();
#endif
}

JPH::BodyID Physics::AddBody(JPH::BodyCreationSettings bodySettings, JPH::EActivation activation)
{
    JPH::BodyID id = m_bodyInterface->CreateAndAddBody(bodySettings, activation);
    m_bodyIDs.emplace_back(id);
    m_bodyInterface->ActivateBody(id);

    return id;
}

void Physics::SetPosition(JPH::BodyID id, const JPH::Vec3 &velocity)
{
    m_bodyInterface->SetPosition(id, velocity, JPH::EActivation::Activate);
}

void Physics::OptimizeBroadphase()
{
    m_physicsSystem.OptimizeBroadPhase();
}

void Physics::AddVelocity(JPH::BodyID id, const JPH::Vec3& velocity)
{
    m_bodyInterface->AddLinearVelocity(id, velocity);
}

void Physics::SetVelocity(JPH::BodyID id, const JPH::Vec3& velocity)
{
    m_bodyInterface->SetLinearVelocity(id, velocity);
}

void Physics::SetRotation(JPH::BodyID id, const JPH::Vec3 &rotation)
{
    m_bodyInterface->SetRotation(id, JPH::Quat::sEulerAngles(rotation), JPH::EActivation::Activate);
}

void Physics::RemoveBody(JPH::BodyID& id)
{
    for (int i = 0; i < m_bodyIDs.size(); i++)
    {
        if (m_bodyIDs[i] == id)
            m_bodyIDs.erase(m_bodyIDs.begin() + i);
    }

    m_bodyInterface->RemoveBody(id);
}


void Physics::CharacterHandler::Init(JPH::PhysicsSystem &physicsSystem)
{
    JPH::CapsuleShapeSettings characterShapeSettings{0.32, 0.3};
    characterShapeSettings.SetEmbedded();

    JPH::ShapeRefC characterShape = characterShapeSettings.Create().Get();

    JPH::CharacterSettings characterSettings;
    characterSettings.SetEmbedded();
    characterSettings.mLayer = JPHImpls::ObjectLayers::MOVING;
    characterSettings.mShape = characterShape;

    characterSettings.mEnhancedInternalEdgeRemoval = true;
    characterSettings.mFriction = 0.7f;

    m_character = new JPH::Character{
        &characterSettings, {-35, 2, 0},
        JPH::Quat::sIdentity(), 100, &physicsSystem
    };

    m_character->AddToPhysicsSystem();
}

void Physics::CharacterHandler::AddVelocity(JPH::Vec3Arg velocity)
{
    m_character->AddLinearVelocity(velocity);
}

void Physics::CharacterHandler::UpdateCharacter()
{
    //We want to clamp the velocity here but clamp y velocity separately
    //otherwise going up or falling down limits x and z movement
    //and this feels unnatural and generally awful

    //TODO: Clamp only velocity given by the movement keys, instead of all velocity
    JPH::Vec3 velocity = m_character->GetLinearVelocity();
    JPH::Vec3 nonVerticalVelocity = m_character->GetLinearVelocity();
    JPH::Vec3 clampedVelocity = velocity;

    nonVerticalVelocity.SetY(0);

    if (nonVerticalVelocity.Length() > 16)
        clampedVelocity = nonVerticalVelocity.Normalized() * 16;

    clampedVelocity.SetY(std::clamp(velocity.GetY(), -16.f, 16.f));
    m_character->SetLinearVelocity(clampedVelocity);

    m_character->PostSimulation(Constants::maxDistanceFromGroundToStillBeOnTheGround);
}

JPH::RVec3 Physics::CharacterHandler::GetPosition()
{
    return m_character->GetPosition();
}

#ifdef JPH_DEBUG_RENDERER
Physics::DebugRendererImpl::DebugRendererImpl(Shader &shader, const JPH::Vec3& cameraPosition)
:	m_shader(shader),
    m_cameraPosition(cameraPosition)
{
    InitBuffers();
    m_lineVertices.reserve(1'000);
    m_triangleVertices.reserve(10'000);
}


void Physics::DebugRendererImpl::InitBuffers()
{
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);
    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 7, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenVertexArrays(1, &triangleVAO);
    glGenBuffers(1, &triangleVBO);
    glBindVertexArray(triangleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 21, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}


void Physics::DebugRendererImpl::StartFrame()
{
    SetCameraPos({m_cameraPosition[0], m_cameraPosition[1], m_cameraPosition[2]}); //to enable LOD
}


void Physics::DebugRendererImpl::EndFrame()
{
    if (!m_lineVertices.empty())
    {
        glBindVertexArray(lineVAO);
        glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
        glBufferData(
            GL_ARRAY_BUFFER, sizeof(float) * m_lineVertices.size(),
            m_lineVertices.data(), GL_STATIC_DRAW
        );
        glDrawArrays(GL_LINES, 0, m_lineVertices.size() / 7);
        m_lineVertices.clear();
    }

    if (!m_triangleVertices.empty())
    {
        glBindVertexArray(triangleVAO);
        glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
        glBufferData(
            GL_ARRAY_BUFFER, sizeof(float) * m_triangleVertices.size(),
            m_triangleVertices.data(), GL_STATIC_DRAW
        );

        glDrawArrays(GL_TRIANGLES, 0, m_triangleVertices.size() / 7);
        m_triangleVertices.clear();
    }
}


void Physics::DebugRendererImpl::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor)
{
    JPH::Vec4 color = inColor.ToVec4();

    m_lineVertices.insert(
        m_lineVertices.end(), {
        inFrom[0], inFrom[1], inFrom[2],
        color[0], color[1], color[2], color[3],

        inTo[0], inTo[1], inTo[2],
        color[0], color[1], color[2], color[3],
    });
}


void Physics::DebugRendererImpl::DrawTriangle(
    JPH::RVec3Arg inV1, JPH::RVec3Arg inV2,
    JPH::RVec3Arg inV3, JPH::ColorArg inColor, ECastShadow inCastShadow
)
{
    JPH::Vec4 color = inColor.ToVec4();

    m_triangleVertices.insert(
        m_triangleVertices.end(), {
        inV1[0], inV1[1], inV1[2],
        color[0], color[1], color[2], color[3],

        inV2[0], inV2[1], inV2[2],
        color[0], color[1], color[2], color[3],

        inV3[0], inV3[1], inV3[2],
        color[0], color[1], color[2], color[3],
    });
}
#endif
