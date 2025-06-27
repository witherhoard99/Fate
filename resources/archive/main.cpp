#include <chrono>
#include <cmath>
#include <iostream>
#include <vector>
#include <memory>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "../../Renderer.h"
#include "../../Shader.h"
#include "../../Texture.h"

#include "../../Util.h"
#include "../../Input.h"
#include "../../Quads3D.h"

#include <Jolt/Jolt.h>
#include <Jolt/Core/Core.h>

#include "../../Model.h"
#include "../../Physics.h"
#include "../../Player.h"
#include "../../StaticModel.h"
#include "../../StaticObjectFactory.h"

#define ENABLE_IMGUI

#ifdef ENABLE_IMGUI

#	include <imgui/imgui.h>
#	include <imgui/imgui_impl_opengl3.h>
#	include <imgui/imgui_impl_glfw.h>

#endif

int initGL(GLFWwindow*& returnWindow);
void configureOpenGLOptions();
void initJolt();

void openglDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
long double CalculateStandardDeviation(const std::vector<long double>& frameDurations, long double average);

int maina()
{
	std::cout << std::boolalpha;

	GLFWwindow* window;
	int initReturnCode = initGL(window);
	if (initReturnCode != 0)
		return initReturnCode;

	initJolt();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	//We need this extra scope here because otherwise `IndexBuffer` and `VertexBuffer` will be destroyed
	//AFTER `glfwTerminate()` is called. This results in an invalid openGL context, which opengl will complain about.
	//Therefore, we need to destroy them before glfwTerminate() is called
	std::vector<long double> frameDurations;
	std::vector<double> renderClearDurations;
	std::vector<double> physicsUpdateDurations;
	std::vector<double> imguiDurations;
	std::vector<double> gameDrawDurations;
	std::vector<double> swapBuffersDurations;

	frameDurations.reserve(3'000);
	renderClearDurations.reserve(3'000);
	physicsUpdateDurations.reserve(3'000);
	imguiDurations.reserve(3'000);
	gameDrawDurations.reserve(3'000);
	swapBuffersDurations.reserve(3'000);

	{
		Renderer renderer;
		Shader physicsShader{"../resources/shaders/PhysicsVertex.glsl", "../resources/shaders/PhysicsFrag.glsl"};
		Shader lightingShader{"../resources/shaders/LightingVertex.glsl", "../resources/shaders/LightingFrag.glsl"};
		Shader modelShader{"../resources/shaders/ModelVertex.glsl", "../resources/shaders/ModelFrag.glsl"};

		Input input{window};

		//Also we can do this because player does not actually need characterHandler until the main loops starts, and we init the characterHandler before that
		Physics::CharacterHandler* characterHandler = nullptr; //TODO: HACK
		Player player{input, characterHandler};

		/*Texture floorTexture{R"(../resources/images/floor.png)"};
		Texture houseWallsTexture{R"(../resources/images/brick_wall.png)"};
		Texture fancyLightsTexture{R"(../resources/images/white.png)"};

		std::vector<JPH::Mat44> identityModelMatrices {
			JPH::Mat44::sIdentity(),
		};*/

		std::vector<JPH::Mat44> lightsModelMatrices {
			JPH::Mat44::sIdentity(),
		};

		/*std::vector<JPH::Mat44> houseWallsModelMatrices {
			JPH::Mat44::sIdentity(),
			JPH::Mat44::sTranslation(JPH::Vec3(0, 0, -7.2)),
			JPH::Mat44::sRotation(JPH::Vec3(0, 1, 0), JPH::DegreesToRadians(90.f)),
			JPH::Mat44::sTranslation(JPH::Vec3(0, 0, 0)) * JPH::Mat44::sRotation(JPH::Vec3(0, 1, 0), JPH::DegreesToRadians(90.f)),
			JPH::Mat44::sTranslation(JPH::Vec3(0.f, 5.f, -7.f)) * JPH::Mat44::sRotation(JPH::Vec3(1.f, 0.f, 0.f), JPH::DegreesToRadians(90.f)) * JPH::Mat44::sScale(JPH::Vec3(1.f, 7/5.f, 1.f))
		};

		std::vector<JPH::Mat44> personModelMatrices {
			//Legs
			JPH::Mat44::sScale(JPH::Vec3(1.f, 3.f, 1.f)),
			JPH::Mat44::sTranslation(JPH::Vec3(0.6f, 0.f, 0.f)) * JPH::Mat44::sScale(JPH::Vec3(1.f, 3.f, 1.f)),

			//Chest
			JPH::Mat44::sTranslation(JPH::Vec3(-2.4f, 2.5f, 0.f)) * JPH::Mat44::sScale(JPH::Vec3(2.2f, 3.f, 1.f)),

			//Head
			JPH::Mat44::sTranslation(JPH::Vec3(0.85f, 5.f, 0.55f)) * JPH::Mat44::sScale(JPH::Vec3(0.75f, 1.f / 3.f * 1.5f, 0.75f))
		};

		Quads3D floor {
		    -10, -0.001, -10,
			20, 0.001, 20,
			1,
			identityModelMatrices
		};

		Quads3D houseWalls {
			0, 0, 0,
			7, 5, 0.2,
			static_cast<int>(houseWallsModelMatrices.size()),
			houseWallsModelMatrices
		};

		Quads3D person {
			2, 0, 2,
			0.5, 0.75, 0.5,
			static_cast<int>(personModelMatrices.size()),
			personModelMatrices
		};*/

		Quads3D fancyLights {
            0, 0, 0,
            1, 1, 1,
            1,
			lightsModelMatrices
        };

		//<Texture>
		/*uint floorTextureSlot = 1;
		uint houseWallsTextureSlot = 2;
		uint fancyLightsTextureSlot = 3;

		floorTexture.Bind(floorTextureSlot);
		houseWallsTexture.Bind(houseWallsTextureSlot);
		fancyLightsTexture.Bind(fancyLightsTextureSlot);*/
		//<Texture>

		//<MVP>
		JPH::Mat44 projMatrix = JPH::Mat44::sPerspective(glm::radians(75.f), 2560.f/1440.f, 0.1f, 1000.f);
		JPH::Mat44 viewMatrix = player.GetViewMatrix();
		float lightPosition[3]{0, 0, 0};
		//<MVP>

		Physics physics{physicsShader, renderer, projMatrix, viewMatrix, player.GetPosition()};
		player.SetCharacterHandler(physics.GetCharacterHandler());

		StaticModel osaka{
			renderer,
			"../resources/models/city/scene.gltf",
			physics,
		};

		int keys[] {
			GLFW_KEY_W,
			GLFW_KEY_A,
			GLFW_KEY_S,
			GLFW_KEY_D,
			GLFW_KEY_SPACE,
			GLFW_KEY_LEFT_CONTROL,
			GLFW_KEY_LEFT_SHIFT,
			GLFW_KEY_ESCAPE
		};

		input.EnableRawMotion();
		input.RegisterArrayOfKeys({keys});
		input.StartReadingInput();

		double deltaTime = 1000 / 60.0;
		bool drawDebugPhysics = false;

		//<ImGui>
		//This needs to be after we start reading input because we need to register our callbacks before ImGui
#ifdef ENABLE_IMGUI
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui_ImplGlfw_InitForOpenGL(window, true);

		ImGui::StyleColorsDark();

		const char* glsl_version = "#version 430";
		ImGui_ImplOpenGL3_Init(glsl_version);
#endif
		//<ImGui>

		int frames = 0;

		lightingShader.Bind();



		while (!glfwWindowShouldClose(window))
		{
			// std::cout << "Frame: " << frames << "\t" << std::flush;

			auto start = std::chrono::high_resolution_clock::now();

			frames++;

			auto t0 = std::chrono::high_resolution_clock::now();
			renderer.Clear();
			auto t1 = std::chrono::high_resolution_clock::now();
			renderClearDurations.emplace_back(std::chrono::duration<double, std::milli>(t1 - t0).count());


			// std::cout << "Cleared Renderer\t";

#ifdef ENABLE_IMGUI
			t0 = std::chrono::high_resolution_clock::now();
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			t1 = std::chrono::high_resolution_clock::now();
			imguiDurations.emplace_back(std::chrono::duration<double, std::milli>(t1 - t0).count());

			// std::cout << "New ImGui Frame\t";
#endif

			player.ProcessInput(deltaTime);
			viewMatrix = player.GetViewMatrix();

			t0 = std::chrono::high_resolution_clock::now();
			physics.Update(static_cast<float>(deltaTime));
			t1 = std::chrono::high_resolution_clock::now();
			physicsUpdateDurations.emplace_back(std::chrono::duration<double, std::milli>(t1 - t0).count());

			// std::cout << "Frame: " << frames << "Updated physics\t" << std::flush;

			if (drawDebugPhysics)
				goto skipDrawingMainGame;

			t0 = std::chrono::high_resolution_clock::now();

			lightingShader.Bind();
			lightingShader.SetUniform("u_viewMatrix", viewMatrix);
			lightingShader.SetUniform("u_lightPos", {lightPosition[0], lightPosition[1], lightPosition[2]});
			lightingShader.SetUniform("u_enableLighting", true);

			fancyLights.Draw(lightingShader, renderer, projMatrix, viewMatrix);

			modelShader.Bind();
			modelShader.SetUniform("u_lightPos", {lightPosition[0], lightPosition[1], lightPosition[2]});
			modelShader.SetUniform("u_enableLighting", true);

			osaka.Draw(modelShader, projMatrix, viewMatrix);

			t1 = std::chrono::high_resolution_clock::now();
			gameDrawDurations.emplace_back(std::chrono::duration<double, std::milli>(t1 - t0).count());

			// std::cout << "Drew everything to screen\t" << std::flush;

			modelShader.Unbind();

			skipDrawingMainGame:

#ifdef ENABLE_IMGUI
			{
				t0 = std::chrono::high_resolution_clock::now();
				ImGui::Begin("Control");

				if (ImGui::Button("Switch Drawing"))
					drawDebugPhysics = !drawDebugPhysics;

				if (!drawDebugPhysics)
				{
					ImGui::SliderFloat3("Light Position", lightPosition, -50.f, 50.f);
					lightsModelMatrices[0] = JPH::Mat44::sTranslation({lightPosition[0], lightPosition[1], lightPosition[2]});
				}

				ImGui::Text("drawDebugPhysics: %s", drawDebugPhysics ? "true" : "false");
				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
				ImGui::End();

				t1 = std::chrono::high_resolution_clock::now();
			}

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			imguiDurations.back() += std::chrono::duration<double, std::milli>(t1 - t0).count();

			// std::cout << "Did ImGUI stuff\t" << std::flush;
#endif

			t0 = std::chrono::high_resolution_clock::now();
			glfwSwapBuffers(window);
			glfwPollEvents();
			t1 = std::chrono::high_resolution_clock::now();
			swapBuffersDurations.emplace_back(std::chrono::duration<double, std::milli>(t1 - t0).count());

			// std::cout << "Swapped buffers and polled events\t" << std::flush;

			auto end = std::chrono::high_resolution_clock::now();
			deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0;
			frameDurations.emplace_back(deltaTime);

			if (player.GetExitProgram())
				break;

			// std::cout << "End Frame\n" << std::flush;
		}

#ifdef ENABLE_IMGUI
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
#endif
	}

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

	printStats("Render Clear", renderClearDurations);
	printStats("Physics Update", physicsUpdateDurations);
	printStats("Game Draw", gameDrawDurations);
	printStats("ImGui", imguiDurations);
	printStats("Swap Buffers", swapBuffersDurations);

	//Calculate average frame time, and remove the first 5% of frames
	long double averageFrameTime{0};
	long double minFrameTime{0}, maxFrameTime{0};
	long double stdDevFrameDuration{0};

	//Remove the fist and last 2 seconds
	if (frameDurations.size() > 250)
	{
		frameDurations.erase(frameDurations.cbegin(), frameDurations.cbegin() + 120);
		frameDurations.erase(frameDurations.cend() - 120, frameDurations.cend());
	}

	for (const long double& frameDuration : frameDurations)
	{
		averageFrameTime += frameDuration;

		if (frameDuration < minFrameTime)
			minFrameTime = frameDuration;
		if (frameDuration > maxFrameTime)
			maxFrameTime = frameDuration;
	}

	averageFrameTime /= frameDurations.size();
	stdDevFrameDuration = CalculateStandardDeviation(frameDurations, averageFrameTime);

	std::cout << "\n\nNumber of frames: " << frameDurations.size() << '\n';
	std::cout << "Average frame time: " << averageFrameTime << "ms\n";
	std::cout << "Min frame time: " << minFrameTime << "ms\n";
	std::cout << "Max frame time: " << maxFrameTime << "ms\n";
	std::cout << "Standard Deviation: " << stdDevFrameDuration << "\n" << std::endl;

	return Util::terminate(false, 0, "Game Finished", window, true);
}

/**
 * @brief Initializes OpenGL and creates a GLFW window.
 *
 * This function initializes the GLFW library, creates a window with the specified
 * options, and sets up the OpenGL context. It also initializes GLEW to manage
 * OpenGL extensions.
 *
 * @param[out] returnWindow A pointer that will store the created GLFW window.
 * @return 0 on success, or a negative error code on failure.
 *
 * @errors Possible errors include:
 * - Failed to initialize GLFW.
 * - Failed to create the GLFW window.
 * - Failed to initialize GLEW.
 *
 * @remark The function sets the OpenGL context version and profile based on the
 *         options specified in the global `options` variable.
 */
int initGL(GLFWwindow*& returnWindow)
{
	if (!glfwInit())
		return Util::terminate(true, -1, "Failed to initialize GLFW", nullptr, false);

#ifdef _DEBUG
	std::cout << "DEBUG MODE!\n\n";
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, Util::Options::openGLVersionMajor);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, Util::Options::openGLVersionMinor);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	if (Util::options.numMSAASamples > 1)
		glfwWindowHint(GLFW_SAMPLES, Util::options.numMSAASamples);

	GLFWwindow* window = glfwCreateWindow(Util::options.scrWidth,
										  Util::options.scrHeight,
										  Util::options.windowTitle,
										  Util::options.fullscreen ? glfwGetPrimaryMonitor() : nullptr,
										  nullptr);

	if (window == nullptr)
		return Util::terminate(true, -1, "Window could not be created.");

	returnWindow = window;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(Util::options.vsync ? 1 : 0);

	auto glewCode = glewInit();

	if (glewCode != GLEW_OK && glewCode != 4 )
		return Util::terminate(true, -1, "Failed to initialize GLEW", window);

#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(openglDebugCallback, nullptr);
#endif

	glViewport(0, 0, Util::options.scrWidth, Util::options.scrHeight);

	if (Util::options.numMSAASamples > 1)
		glEnable(GL_MULTISAMPLE);

	return 0;
}

inline static void TraceImpl(const char *inFMT, ...)
{
	// Format the message
	va_list list;
	va_start(list, inFMT);
	char buffer[2048];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);
	va_end(list);

	std::cerr << buffer << std::endl;
	JPH_ASSERT(false);
}


inline static bool AssertFailedImpl(const char *inExpression, const char *inMessage, const char *inFile, uint inLine)
{
	std::cerr << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr? inMessage : "") << std::endl;
	return true;
}


void initJolt()
{
	JPH::RegisterDefaultAllocator();
	JPH::Trace = TraceImpl;
	JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl;)

	JPH::Factory::sInstance = new JPH::Factory();
	JPH::RegisterTypes();

#ifdef JPH_PROFILE_ENABLED
	JPH::Profiler::sInstance = new JPH::Profiler();
#endif
}



long double CalculateStandardDeviation(const std::vector<long double>& frameDurations, long double average)
{
	long double sum = 0;
	for (const long double& frameDuration : frameDurations)
		sum += std::pow(frameDuration - average, 2);

	return std::sqrt(sum / frameDurations.size());
}


void openglDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
	const char* severityStr = (severity == GL_DEBUG_SEVERITY_HIGH) ? "High" :
							  (severity == GL_DEBUG_SEVERITY_MEDIUM) ? "Medium" :
							  (severity == GL_DEBUG_SEVERITY_LOW) ? "Low" :
							  "Notification";

	std::cerr << "\nOPENGL ERROR:\n";

	std::cerr << "Source: " << source << std::endl;
	std::cerr << "Type: " << type << std::endl;
	std::cerr << "ID: " << id << std::endl;
	std::cerr << "Severity: " << severityStr << std::endl;
	std::cerr << message << std::endl;

	std::cerr << "\nEXITING using `__debugbreak()`" << std::endl;
	ASSERT_LOG(false, "OpenGL Error, ur cooked!");
}