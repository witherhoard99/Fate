#ifndef INPUT_H
#define INPUT_H

#include "Util.h"
#include <map>
#include <span>

class Input {
private:
	enum class KeyState : uint8
	{
		pressed,
		released,
		held,
		unknown
	};

	struct KeyStateInfo
	{
		KeyState m_state;

		KeyStateInfo()
		{
			m_state = KeyState::unknown;
		}

		void ChangeState(KeyState newState);

	private:
		inline void ChangeState_(KeyState newState)
		{
			m_state = newState;
		}
	};

	bool m_isMouseNormalMode = false;

	GLFWwindow* m_window;
	std::map<int, KeyStateInfo> m_keys; //Unordered map is slower when less than 100 elements

	//This exists as a workaround because the cursor does not get hidden on linux (might be wayland specefic)
	//So instead, we display the standard cursor when we want a cursor and display a NULL cursor when we don't want a cursor
	//This works just fine on windows (even though we do not need it) so we are not going to #ifdef this
	GLFWcursor* m_standardCursor;

	float m_mouseX;
	float m_mouseY;
	float m_mouseXDelta;
	float m_mouseYDelta;

	void static glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void static glfwMouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
	void static glfwCursorPosCallback(GLFWwindow* window, double xpos, double ypos);

public:

	Input();
	explicit Input(GLFWwindow* window);
	~Input();

	void Init(GLFWwindow* window);

	/**
	 * @brief Starts reading input from the window.
	 *
	 * This function sets the key and mouse callback for the window to start reading input.
	 */
	void StartReadingInput();

	/**
	 * Is the key being pressed? The key can be repeating or pressed for the first time (both situations
	 * cause this method to return true)
	 * @param key the GLFW key code
	 */
	bool IsKeyCurrentlyPressed(int key);

	/**
	 * @brief Registers an array of keys (GLFW key codes) for input handling.
	 * Keys not registered will be ignored by input methods.
	 *
	 * @param keysArray An array, represented by a std::span
	 */
	void RegisterArrayOfKeys(std::span<int> keysArray);

	/**
	 * @brief Toggles the mouse enabled state.
	 *
	 * This function enables or disables the mouse cursor.
	 */
	void FlipMouseEnabled();

	/**
	 * @brief Enables raw mouse motion.
	 *
	 * This function enables raw mouse motion if supported, ASSERTS.
	 */
	void EnableRawMotion();

	/**
	 * @brief Resets the mouse deltas
	 */
	void NewFrame();

	const float& GetMouseX()		const { return m_mouseX; }
	const float& GetMouseY()		const { return m_mouseY; }
	const float& GetMouseXDelta()	const { return m_mouseXDelta; }
	const float& GetMouseYDelta()	const { return m_mouseYDelta; }
	const bool&  GetIsMouseNormal()	const { return m_isMouseNormalMode; }
};



#endif //INPUT_H
