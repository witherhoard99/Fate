#include "Texture.h"
#include <stb_image/stb_image.h>

Texture::Texture()
	:	m_rendererID(-1),
		m_width(0),
		m_height(0),
		m_bytesPerPixel(0),
		m_filepath("NULL PATH!!"),
		m_texType(TextureType::unknown)
{}

Texture::Texture(const std::string &path, TextureType texType, bool flipVertically, bool mip)
	:	m_rendererID(-1),
		m_width(0),
		m_height(0),
		m_bytesPerPixel(0)
{
	Init(path, texType, flipVertically, mip);
}

Texture::~Texture()
{
	glDeleteTextures(1, &m_rendererID);
	m_rendererID = -1; //Yes this wraps around
}

void Texture::Init(const std::string &path, TextureType texType, bool flipVertically, bool mip)
{
	m_filepath = path;
	m_texType = texType;

	stbi_set_flip_vertically_on_load(flipVertically);
	unsigned char* buffer = stbi_load(path.c_str(), &m_width, &m_height, &m_bytesPerPixel, 4);

	glGenTextures(1, &m_rendererID);
	glBindTexture(GL_TEXTURE_2D, m_rendererID);

	if (mip)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if (buffer)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

		if (mip)
			glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(buffer);
	}
	else
	{
		std::cerr << "[ERROR, Texture.cpp, Constructor] Failed to load texture with path: \"" << path << "\"" << std::endl;
		std::cerr << "STBI Error message: " << stbi_failure_reason() << std::endl;
		ASSERT_LOG(false, "Texture could not be created");
	}
}

void Texture::Bind(uint slot /* = 0 */) const
{
	//We can add to get to the correct slot because the #defines in opengl are consecutive integers
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_rendererID);
}

void Texture::Unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

std::string Texture::ConvertTextureTypeToString(TextureType texType)
{
	if (texType == TextureType::specular)
		return "Specular";
	if (texType == TextureType::diffuse)
		return "Diffuse";
	ASSERT_LOG(false, "Tex type is not of known type! Invalid texType!");
	return "INVALID TEXTURE!!";
}
