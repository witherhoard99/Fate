#include "Input.h"
#include "Constants.h"

#include <stdexcept>

Input::Input()
	:	m_window(nullptr),
		m_standardCursor(glfwCreateStandardCursor(GLFW_ARROW_CURSOR))
{
}

Input::Input(GLFWwindow* window)
	:	m_window(window),
		m_standardCursor(glfwCreateStandardCursor(GLFW_ARROW_CURSOR))
{
	glfwSetWindowUserPointer(window, this);
}

Input::~Input()
{
	glfwDestroyCursor(m_standardCursor);
}

void Input::Init(GLFWwindow *window)
{
	glfwSetWindowUserPointer(window, this);
	//m_standardCursor will already be set by the default constructor
}

void Input::KeyStateInfo::ChangeState(KeyState newState)
{
	ASSERT_LOG(newState != KeyState::unknown, "Invalid newState of:" << static_cast<int>(newState));
	ASSERT_LOG(m_state != KeyState::held, "Key state should not be set to held");

	if (m_state == KeyState::pressed)
	{
		switch (newState)
		{
			case KeyState::pressed:
				ChangeState_(KeyState::pressed);
				return;

			case KeyState::released:
				ChangeState_(newState);
				return;

			case KeyState::held:
				ChangeState_(KeyState::pressed);
				return;
		}
	}

	if (m_state == KeyState::released)
	{
		switch (newState)
		{
			case KeyState::released:
				return;

			case KeyState::pressed:
				ChangeState_(newState);
				return;

			case KeyState::held:
				ChangeState_(KeyState::pressed);
				return;
		}
	}

	if (m_state == KeyState::unknown)
		ChangeState_(newState);
}


void Input::glfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	auto instance = static_cast<Input*>(glfwGetWindowUserPointer(window));
	if (!instance->m_keys.contains(key))
		return;

	if (action == GLFW_REPEAT)
	{
		instance->m_keys[key].ChangeState(KeyState::held);
	}
	else if (action == GLFW_PRESS)
	{
		instance->m_keys[key].ChangeState(KeyState::pressed);
	}
	else if (action == GLFW_RELEASE)
	{
		instance->m_keys[key].ChangeState(KeyState::released);
	}
}

void Input::glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	auto instance = static_cast<Input*>(glfwGetWindowUserPointer(window));
	if (!instance->m_keys.contains(button))
		return;

	if (action == GLFW_PRESS)
	{
		instance->m_keys[button].ChangeState(KeyState::pressed);
	}
	else if (action == GLFW_RELEASE)
	{
		instance->m_keys[button].ChangeState(KeyState::released);
	}
}

void Input::glfwCursorPosCallback(GLFWwindow *window, double xpos, double ypos)
{
	auto &instance = *static_cast<Input*>(glfwGetWindowUserPointer(window));

	// Convert xpos and ypos to the range [-1, 1]
	int width, height;
	glfwGetWindowSize(window, &width, &height);

	instance.m_mouseXDelta = (xpos - instance.m_mouseX) / width * 2.0f;
	instance.m_mouseYDelta = (instance.m_mouseY - ypos) / height * 2.0f;

	if (instance.m_mouseXDelta < Constants::DEADZONE && instance.m_mouseXDelta > -Constants::DEADZONE)
		instance.m_mouseXDelta = 0;

	if (instance.m_mouseYDelta < Constants::DEADZONE && instance.m_mouseYDelta > -Constants::DEADZONE)
		instance.m_mouseYDelta = 0;

	instance.m_mouseX = xpos;
	instance.m_mouseY = ypos;
}

void Input::StartReadingInput()
{
	glfwSetKeyCallback(m_window, glfwKeyCallback);
	glfwSetMouseButtonCallback(m_window, glfwMouseButtonCallback);
	glfwSetCursorPosCallback(m_window, glfwCursorPosCallback);
}

bool Input::IsKeyCurrentlyPressed(int key)
{
	if (!m_keys.contains(key))
		return false;

	return m_keys[key].m_state == KeyState::pressed;
}

void Input::RegisterArrayOfKeys(std::span<int> keysArray)
{
	for (int i = 0; i < keysArray.size(); i++)
    {
        m_keys[keysArray[i]] = {};
    }
}

void Input::FlipMouseEnabled()
{
    m_isMouseNormalMode = !m_isMouseNormalMode;
	glfwSetInputMode(m_window, GLFW_CURSOR, m_isMouseNormalMode ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);

	if (!m_isMouseNormalMode)
		glfwSetCursor(m_window, NULL);
	else
		glfwSetCursor(m_window, m_standardCursor);
}

void Input::EnableRawMotion()
{
    if (!glfwRawMouseMotionSupported())
    	ASSERT_LOG(false, "Raw mouse motion not supported");

	glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
}

void Input::NewFrame()
{
	m_mouseXDelta = 0;
	m_mouseYDelta = 0;
}

