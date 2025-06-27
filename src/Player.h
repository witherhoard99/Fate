#ifndef PLAYER_H
#define PLAYER_H

#include "Audio.h"
#include "Constants.h"
#include "Input.h"
#include "../Physics.h"

class Boss;

class Player
{
private:
	float m_mouseSensitivityX;
	float m_mouseSensitivityY;

	Input &m_input;
	Physics::CharacterHandler* m_characterHandler;

	JPH::Vec3 m_position;
	JPH::Vec3 m_front;
	JPH::Vec3 m_up;
	JPH::Vec3 m_right;
	const JPH::Vec3 m_worldUp;

	float m_yaw;
	float m_pitch;

	Audio m_shootSound;
	Audio m_hitSound;

	std::random_device m_randDevice;
	std::mt19937 m_randEngine{m_randDevice()};
	std::uniform_real_distribution<float> m_recoilYDistribution{Constants::RECOIL_Y_MIN, Constants::RECOIL_Y_MAX};
	std::uniform_real_distribution<float> m_recoilXDistribution{Constants::RECOIL_X_MIN, Constants::RECOIL_X_MAX};

	//TODO: Get rid of benchmark code
	std::chrono::time_point<std::chrono::system_clock> startTime = std::chrono::high_resolution_clock::now();
	bool exitProgram = false;

	void UpdateCameraVectors();

	void ProcessKeyboard(float deltaTime);
	void ProcessMouseMovement(float deltaTime);

	void ProcessInput(float deltaTime);

public:
	Player(
		Input &input, Physics::CharacterHandler* characterHandler,
		JPH::Vec3 position = JPH::Vec3(0.0f, 0.5f, 1.0f),
		JPH::Vec3 up = JPH::Vec3(0.0f, 1.0f, 0.0f),
		float yaw = Constants::STARTING_YAW,
		float pitch = Constants::STARTING_PITCH
	);

	//Must be called before using any other methods. This is to allow delayed initialization
	void SetCharacterHandler(Physics::CharacterHandler* characterHandler) { m_characterHandler = characterHandler; }

	float GetDamageAmountForCurrrentGun() { return 0.1f; }

	//Should be called every frame to processInput and get hits
	void Update(Boss* boss, int numBosses, float deltaTime);

	const JPH::Vec3& GetPosition() const { return m_position; }
	JPH::Vec3 GetAimVector() const { return m_front; }

	bool GetExitProgram() const;

	JPH::Mat44 GetViewMatrix() const
	{
		return JPH::Mat44::sLookAt(m_position, m_position + m_front, m_up);
	}
};



#endif //PLAYER_H
