#include "Shader.h"
#include <glm/gtc/type_ptr.hpp>
#include <fstream>

Shader::Shader(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath)
{
	std::string vertexShader = ParseShader(vertexShaderFilePath);
	std::string fragmentShader = ParseShader(fragmentShaderFilePath);

	int shaderIDResult = CreateShaders(vertexShader, fragmentShader);

	if (shaderIDResult == -1)
		ASSERT_LOG(false, "Shader could not be created.")

	m_RendererID = static_cast<uint>(shaderIDResult);
}

Shader::~Shader()
{
	glDeleteProgram(m_RendererID);
}

int Shader::CreateShaders(const std::string& vertexShader, const std::string& fragmentShader)
{
	uint programID = glCreateProgram();
	int vertexShaderID = CompileShader(GL_VERTEX_SHADER, vertexShader);
	int fragmentShaderID = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

	if (vertexShaderID == -1 || fragmentShaderID == -1 )
	{
		glDeleteShader(vertexShaderID);
		glDeleteShader(fragmentShaderID);
		glDeleteProgram(programID);
		return -1;
	}

	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);
	glValidateProgram(programID);

	//We can delete them because they have been attached to a program
	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	return static_cast<int>(programID);
}

int Shader::CompileShader(uint type, const std::string& shaderSource)
{
	uint shaderID = glCreateShader(type);
	const char* shaderSrc = shaderSource.c_str();

	glShaderSource(shaderID, 1, &shaderSrc, nullptr);
	glCompileShader(shaderID);

	int result;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);

	if (result == GL_FALSE)
	{
		int length;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &length);

		char* message = (char*) alloca(length * sizeof(char));
		glGetShaderInfoLog(shaderID, length, &length, message);

		std::cerr << "Failed to compile shader of type: " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << "\n";
		std::cerr << message << "\n";

		glDeleteShader(shaderID);
		return -1;
	}

	return static_cast<int>(shaderID);
}

std::string Shader::ParseShader(const std::string& filepath)
{
	std::ifstream file(filepath);
	std::string str;
	std::string content;

	while (std::getline(file, str)) {
		content.append(str + "\n");
	}

	return content;
}

void Shader::Bind() const
{
	glUseProgram(m_RendererID);
}

void Shader::Unbind()
{
	glUseProgram(0);
}

void Shader::SetUniform(const std::string& uniformName, float val)
{
	glUniform1f(GetUniformLocation(uniformName), val);
}

void Shader::SetUniform(const std::string& uniformName, int val)
{
	glUniform1i(GetUniformLocation(uniformName), val);
}

void Shader::SetUniform(const std::string& uniformName, uint val)
{
	glUniform1i(GetUniformLocation(uniformName), val);
}

void Shader::SetUniform(const std::string &uniformName, bool val)
{
	glUniform1i(GetUniformLocation(uniformName), val);
}

void Shader::SetUniform(const std::string& uniformName, const JPH::Vec3& val)
{
	glUniform3f(GetUniformLocation(uniformName), val[0], val[1], val[2]);
}

void Shader::SetUniform(const std::string& uniformName, const JPH::Vec4& val)
{
	glUniform4f(GetUniformLocation(uniformName), val[0], val[1], val[2], val[3]);
}

void Shader::SetUniform(const std::string &uniformName, const JPH::Mat44 &val)
{
	//Because JPH::Mat44 is guaranteed to be a trivial type, we can just reinterpret_cast safely
	glUniformMatrix4fv(GetUniformLocation(uniformName), 1, GL_FALSE, reinterpret_cast<const float*>(&val));
}

int Shader::GetUniformLocation(const std::string& uniformName)
{
	if (m_uniformLocationMap.contains(uniformName))
		return m_uniformLocationMap[uniformName];

	int uniformLocation = glGetUniformLocation(m_RendererID, uniformName.c_str());
	if (uniformLocation == -1)
		std::clog << "[WARNING] Uniform \"" << uniformName << "\" requested but not found - Shader.cpp, GetUniformLocation" << std::endl;

	m_uniformLocationMap[uniformName] = uniformLocation;

	return uniformLocation;
}
