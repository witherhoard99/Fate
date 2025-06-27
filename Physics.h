#ifndef JOLTPHYSICSIMPLS_H
#define JOLTPHYSICSIMPLS_H

#include "Util.h"
#include "Renderer.h"
#include "Shader.h"

#include <cstdarg>
#include <thread>

#include <Jolt/Jolt.h>
#include <Jolt/Core/Core.h>
#include <Jolt/Core/Profiler.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

#ifdef JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRendererSimple.h>
#endif

#include "Core/JobSystemSingleThreaded.h"
#include "Physics/Collision/Shape/CapsuleShape.h"
#include "Physics/Character/Character.h"

namespace ObjectLayers
{
	static constexpr JPH::ObjectLayer NON_MOVING = 0;
	static constexpr JPH::ObjectLayer MOVING = 1;
	static constexpr JPH::ObjectLayer FLOOR = 2;

	static constexpr JPH::ObjectLayer NUM_LAYERS = 3;
}

namespace BroadPhaseLayers
{
	static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
	static constexpr JPH::BroadPhaseLayer MOVING(1);
	static constexpr JPH::uint NUM_LAYERS(2);
}

class Physics
{
private:

	class ObjectLayerPairCollisionFilterImpl : public JPH::ObjectLayerPairFilter
	{
	public:
		bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::ObjectLayer inLayer2) const override;
	};



	class BPLayerInterfaceImpl : public JPH::BroadPhaseLayerInterface
	{
	public:
		BPLayerInterfaceImpl();
		uint GetNumBroadPhaseLayers() const override;
		JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override;

#		if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
			const char * GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override;
#		endif

	private:
		JPH::BroadPhaseLayer mObjectToBroadPhase[ObjectLayers::NUM_LAYERS];

	};



	class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
	{
	public:
		bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override;
	};


#ifdef JPH_DEBUG_RENDERER
	class DebugRendererImpl : public JPH::DebugRendererSimple
	{
	private:
		Shader &m_shader;

		std::vector<float> m_lineVertices;
		std::vector<float> m_triangleVertices;

		uint lineVAO, lineVBO;
		uint triangleVAO, triangleVBO;

		const JPH::Vec3& m_cameraPosition;

		void InitBuffers();

	public:
		DebugRendererImpl(Shader &shader, const JPH::Vec3& cameraPosition);

		void StartFrame();
		void EndFrame();

		virtual void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;
		virtual void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, ECastShadow inCastShadow) override;

		virtual void DrawText3D(JPH::RVec3Arg inPosition, const std::string_view &inString, JPH::ColorArg inColor, float inHeight) override
		{
			// Not implemented
		}
	};
#endif


public:
	class CharacterHandler
	{
	private:
		JPH::Ref<JPH::Character> m_character;

	public:
		CharacterHandler() = default;
		void Init(JPH::PhysicsSystem &physicsSystem);

		void AddVelocity(JPH::Vec3Arg velocity);
		void UpdateCharacter();

		JPH::RVec3 GetPosition();

		void Destruct() { m_character->RemoveFromPhysicsSystem(); }
	};
private:


	void MaybeOptimizeBroadPhase()
	{
		if (m_numSingularBodiesAdded > 5)
		{
			m_physicsSystem.OptimizeBroadPhase();
			m_numSingularBodiesAdded = 0;
		}
	}


	BPLayerInterfaceImpl m_BPLayerInterface;
	ObjectVsBroadPhaseLayerFilterImpl m_objVsBPLayerFilter;
	ObjectLayerPairCollisionFilterImpl m_objLayerPairCollisonFilter;

#ifdef JPH_DEBUG_RENDERER
	DebugRendererImpl m_debugRenderer;
	JPH::BodyManager::DrawSettings m_drawSettings;
#endif

	JPH::TempAllocatorImplWithMallocFallback m_tempAllocator;
	JPH::PhysicsSystem m_physicsSystem;
	// JPH::JobSystemThreadPool m_jobSystem;
	JPH::JobSystemSingleThreaded m_jobSystem;
	JPH::BodyInterface *m_bodyInterface;


#	ifdef JPH_PROFILE_ENABLED
		JPH::Profiler* m_profiler;
		JPH::ProfileThread m_profileThread;
#	endif

	std::vector<JPH::BodyID> m_bodyIDs;
	CharacterHandler m_characterHandler;

	int m_numSingularBodiesAdded = 0;

	Shader &m_shader;
	Renderer &m_renderer;
	const JPH::Mat44& m_projMatrix;
	const JPH::Mat44& m_viewMatrix;

public:
	Physics(
		Shader& shader, Renderer& renderer, const JPH::Mat44& projMatrix,
		const JPH::Mat44& viewMatrix, const JPH::Vec3& cameraPosition
	);

	~Physics();

	/**
	 * @param deltaTime delta time in milliseconds
	 */
	void Update(float deltaTime);

	void DrawDebugPhysics();

	JPH::BodyID AddBody(JPH::BodyCreationSettings bodySettings, JPH::EActivation activation = JPH::EActivation::Activate);

	void OptimizeBroadphase();

	CharacterHandler* GetCharacterHandler() { return &m_characterHandler; }
	const JPH::BodyLockInterfaceLocking& GetBodyManager() { return m_physicsSystem.GetBodyLockInterface(); }

	JPH::Vec3 GetPosition(JPH::BodyID id) const { return m_bodyInterface->GetPosition(id); }
	JPH::Quat GetRotation(JPH::BodyID id) const { return m_bodyInterface->GetRotation(id); }

	void SetPosition(JPH::BodyID id, const JPH::Vec3& velocity);
	void AddVelocity(JPH::BodyID id, const JPH::Vec3& velocity);
	void SetVelocity(JPH::BodyID id, const JPH::Vec3& velocity);
	void SetRotation(JPH::BodyID id, const JPH::Vec3& rotation);

	void RemoveBody(JPH::BodyID& id);
};


#endif //JOLTPHYSICSIMPLS_H
