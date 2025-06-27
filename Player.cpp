#include "Util.h"
#include "Player.h"
#include "Input.h"
#include "game/Boss.h"
#include "imgui/imgui.h"

Player::Player(
	Input &input, Physics::CharacterHandler* characterHandler, JPH::Vec3 position,
	JPH::Vec3 up, float yaw, float pitch
) :
	m_mouseSensitivityX(Constants::SENSITIVITY_X),
	m_mouseSensitivityY(Constants::SENSITIVITY_Y),
	m_input(input),
	m_characterHandler(characterHandler),
	m_front(0.0f, 0.0f, -1.0f),
	m_worldUp(up),
	m_shootSound("../resources/audio/m4-fire-loop.mp3", true, 25),
	m_hitSound("../resources/audio/bullet-hit-loop-speed.mp3", true, 75)
{
	m_position = position;
	m_yaw = yaw;
	m_pitch = pitch;

	UpdateCameraVectors();

	int keys[]
	{
		GLFW_KEY_W,
		GLFW_KEY_A,
		GLFW_KEY_S,
		GLFW_KEY_D,
		GLFW_KEY_SPACE,
		GLFW_KEY_LEFT_CONTROL,
		GLFW_KEY_LEFT_SHIFT,
		GLFW_KEY_ESCAPE,

		GLFW_MOUSE_BUTTON_LEFT
	};

	m_input.RegisterArrayOfKeys({keys});
	m_input.EnableRawMotion();
	m_input.StartReadingInput();
}

void Player::ProcessInput(float deltaTime)
{
	if (m_input.IsKeyCurrentlyPressed(GLFW_KEY_ESCAPE))
		m_input.FlipMouseEnabled();

	if (m_input.GetIsMouseNormal())
		return;

	ProcessKeyboard(deltaTime);
	ProcessMouseMovement(deltaTime);

	m_input.NewFrame();
}

void Player::Update(Boss* boss, int numBosses, float deltaTime)
{
	ProcessInput(deltaTime);
	
	if (m_input.IsKeyCurrentlyPressed(GLFW_MOUSE_BUTTON_LEFT))
	{
		m_pitch += m_recoilYDistribution(m_randEngine) * deltaTime;
		m_yaw += m_recoilXDistribution(m_randEngine) * deltaTime;

		m_shootSound.ContinuePlaying();
	}
	else
	{
		m_shootSound.Pause();
		m_hitSound.Stop();
		UpdateCameraVectors();
		return;
	}

	bool hit = false;
	for (int i = 0; i < numBosses; i++)
	{
		hit = boss[i].CheckForHit(*this, deltaTime);

#		ifdef ENABLE_IMGUI
			if (hit)
				ImGui::Text("Hit!: %s", "true!");
			else
				ImGui::Text("Hit!: %s", "false!");

			ImGui::Text("Boss Health: %f", boss[i].GetHealth());
#		endif

		if (hit)
			break;
	}

	if (hit)
	{
		m_hitSound.ContinuePlaying();
	}
	else
	{
		m_hitSound.Stop();
	}

	UpdateCameraVectors();
}

bool Player::GetExitProgram() const
{
	if constexpr (Constants::benchmark)
		return exitProgram;

	return false;
}

void Player::ProcessKeyboard(float deltaTime)
{
	bool forward = m_input.IsKeyCurrentlyPressed(GLFW_KEY_W);
	bool backward = m_input.IsKeyCurrentlyPressed(GLFW_KEY_S);
	bool left = m_input.IsKeyCurrentlyPressed(GLFW_KEY_A);
	bool right = m_input.IsKeyCurrentlyPressed(GLFW_KEY_D);
	bool up = m_input.IsKeyCurrentlyPressed(GLFW_KEY_SPACE);
	bool down = m_input.IsKeyCurrentlyPressed(GLFW_KEY_LEFT_CONTROL);
	bool sprint = m_input.IsKeyCurrentlyPressed(GLFW_KEY_LEFT_SHIFT);

	if constexpr (Constants::benchmark)
	{
		using namespace std::chrono;

		auto currentTime = high_resolution_clock::now();
		auto secondsSinceBeginning = duration_cast<milliseconds>(currentTime - startTime).count() / 1000.0;

		if (secondsSinceBeginning < 2.5)
			m_position = {0, 5, 0};

		if (secondsSinceBeginning > 2.5 && secondsSinceBeginning < 5)
			m_position = {0, 20, 0};

		if (secondsSinceBeginning > 5 && secondsSinceBeginning < 7.5)
			m_position = {25, 5, 25};

		if (secondsSinceBeginning > 7.5 && secondsSinceBeginning < 10)
			m_position = {25, 20, 25};

		if (secondsSinceBeginning > 10 && secondsSinceBeginning < 12.5)
			m_position = {-25, 5, 25};

		if (secondsSinceBeginning > 12.5 && secondsSinceBeginning < 15)
			m_position = {-25, 20, 25};

		if (secondsSinceBeginning > 15)
			exitProgram = true;

		return;
	}

	skip: //TODO: Get rid of the benchmark code and the goto

	float movementSpeed = sprint ? Constants::SPEED * Constants::SPRINT_SPEED_MULTIPLIER : Constants::SPEED;
	float velocity = movementSpeed * deltaTime;

	JPH::Vec3 front{m_front[0], 0, m_front[2]};
	front = front.Normalized();

	JPH::Vec3 combinedVelocity{0, 0, 0};

	if (forward)
		combinedVelocity += front * velocity;
	if (backward)
		combinedVelocity -= front * velocity;

	if (left)
		combinedVelocity -= m_right * velocity;
	if (right)
		combinedVelocity += m_right * velocity;

	//We always want to go up or down when we press the up or down key, regardless of the camera's orientation
	if (up)
		combinedVelocity += m_worldUp * velocity * 4;
	if (down)
		combinedVelocity -= m_worldUp * velocity * 4;

	if (m_characterHandler == nullptr)
	{
		ASSERT_LOG(false, "Character handler is nullptr!");
		return;
	}

	m_characterHandler->AddVelocity(combinedVelocity);
	m_position = m_characterHandler->GetPosition();
}

void Player::ProcessMouseMovement(float deltaTime)
{
	float xoffset = m_input.GetMouseXDelta() * (deltaTime / 10);
	float yoffset = m_input.GetMouseYDelta() * (deltaTime / 10);

	xoffset *= m_mouseSensitivityX;
	yoffset *= m_mouseSensitivityY;

	m_yaw   += xoffset;
	m_pitch += yoffset;

	// constrain the pitch, making sure that when pitch is out of bounds, screen doesn't get flipped
	if (m_pitch > 89.0f)
		m_pitch = 89.0f;
	if (m_pitch < -89.0f)
		m_pitch = -89.0f;
}

void Player::UpdateCameraVectors()
{
	JPH::Vec3 front;
	front.SetX(cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch)));
	front.SetY(sin(glm::radians(m_pitch)));
	front.SetZ(sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch)));

	m_front = front.Normalized();
	m_right = front.Cross(m_worldUp).Normalized();  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	m_up    = m_right.Cross(front).Normalized();
}
