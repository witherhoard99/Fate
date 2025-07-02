#ifndef GLUTILS_H
#define GLUTILS_H

#ifdef __linux__
	#define __debugbreak() __asm__ volatile("int3")
#endif

#define IF_DEBUG JPH_IF_DEBUG

#define ASSERT_LOG(inExpression, ...)																					\
	do {																												\
		if (!(inExpression)) [[unlikely]]																				\
		{																												\
			std::cerr << "[ERROR, " << __FILE__ << ", " << __LINE__ << "] " <<											\
				#inExpression << ": " << __VA_ARGS__ << "\n";															\
			__debugbreak();																								\
		}																												\
	} while (false);

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <unordered_map>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Jolt.h>
#include <Jolt/Core/Core.h>
#include <Jolt/Core/IssueReporting.h>

using uint = unsigned int;
using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;

using int8 = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;

//Should not be needed but just in case (this forces the compiler to pack all data, ensuring no padding)
#pragma pack(push, 1)
struct vertexUV
{
	float posX;
	float posY;
	float posZ;

	float texcoordX;
	float texcoordY;
};

struct vertexUV2D
{
	float posX;
	float posY;

	float texcoordX;
	float texcoordY;
};

struct vertexUVNormal
{
	float posX;
	float posY;
	float posZ;

	float texcoordX;
	float texcoordY;

	float normalX;
	float normalY;
	float normalZ;
};
#pragma pack(pop)

static_assert(std::is_trivial_v<vertexUV>,  "The vertexUV struct is not trivial");
static_assert(std::is_trivial_v<vertexUVNormal>,  "The vertexUVNormal struct is not trivial");

namespace Util
{
	struct Options
	{
		static constexpr int openGLVersionMajor = 4;
		static constexpr int openGLVersionMinor = 6;

		int scrWidth = 1920;
		int scrHeight = 1080;

		int numMSAASamples = 2; //Set to -1 to disable

		const char* windowTitle = "OpenGL";
		bool vsync = false;
		bool fullscreen = true; //IMPORTANT: Fullscreen causes computer to freeze when using GDB
	};

	inline Options options;

	/**
	* @brief Handles cleaning up resources during termination of the program.
	*
	* This function terminates the program, handling the destruction of the GLFW window and the termination of GLFW,
	* while printing a message to the appropriate output stream.
	*
	* @code
	* // Example Usage
	* return terminate(false, 0, "Game Finished", window);
	* @endcode
	*
	* @param[in] isError If true, the message is printed to std::cerr, otherwise to std::cout.
	* @param[in] returnCode The return code of the method.
	* @param[in] message The message to print to std::cout or std::cerr, depending on @p isError.
	* @param[in] window The window to destroy (optional, if nullptr, @ref glfwDestroyWindow() is not called).
	* @param[in] callGlfwTerminate If true, @ref glfwTerminate() is called (optional, default is true).
	* @return The specified return code.
	*/
	int terminate(bool isError, int returnCode, const std::string& message, GLFWwindow* window = nullptr, bool callGlfwTerminate = true);
	inline int terminate(bool isError, int returnCode, const std::string &message, GLFWwindow *window, bool callGlfwTerminate)
	{
		if (isError)
		{
			std::cerr << message << std::endl;
			__debugbreak();
		}
		else
		{
			std::cout << message << std::endl;
		}

		if (window != nullptr)
			glfwDestroyWindow(window);

		if (callGlfwTerminate)
			glfwTerminate();

		return returnCode;
	}
}

class Error
{
public:
	enum class ErrorCode : uint16
	{
		NoError = 0,
		TooManyTotalTexturesInModel
	};

private:
	ErrorCode m_errorCode;
	bool m_hasError = false;

public:
	Error() = default;
	~Error() noexcept(false)
	{
		if (m_hasError)
		{
			ASSERT_LOG(m_hasError, "Error has not been checked, and there is an error of: " << static_cast<long long>(m_errorCode));
			throw std::runtime_error("Error has not been checked");
		}
	}

	void Set(ErrorCode error)
	{
		m_errorCode = error;
		m_hasError = true;
	}

	bool HasError() const
	{
		return m_hasError;
	}

	ErrorCode GetErrorCode()
	{
		return m_errorCode;
	}

	void* operator new(size_t) = delete;
	void* operator new[](size_t) = delete;
};

#define HANDLE_ERROR(error, errorHandleCode)																			\
	if (error.HasError())																								\
	{																													\
		ASSERT_LOG(false, "We have error of type: " << std::to_string(static_cast<int64>(error.GetErrorCode())));		\
		errorHandleCode;																								\
	}

#define CHECK_ERROR(error) \
	HANDLE_ERROR(error, void(0);)

#endif //GLUTILS_H
