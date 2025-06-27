#ifndef SHADER_H
#define SHADER_H

#include <map>
#include <string>

#include <glm/glm.hpp>

#include "Util.h"

class Shader {
private:
	uint m_RendererID;
	std::map<std::string, int> m_uniformLocationMap;

	int GetUniformLocation(const std::string& name);

	int CreateShaders(const std::string& vertexShader, const std::string& fragmentShader);
	int CompileShader(uint type, const std::string& shaderSource);
	std::string ParseShader(const std::string& filepath);

public:
	Shader(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath);
	~Shader();

	void Bind() const;

	static void Unbind();

	void SetUniform(const std::string& uniformName, float val);
	void SetUniform(const std::string& uniformName, int val);
	void SetUniform(const std::string& uniformName, uint val);
	void SetUniform(const std::string& uniformName, bool val);

	void SetUniform(const std::string& uniformName, const JPH::Vec3& val);
	void SetUniform(const std::string& uniformName, const JPH::Vec4& val);

	void SetUniform(const std::string& uniformName, const JPH::Mat44& val);
};


#endif //SHADER_H
