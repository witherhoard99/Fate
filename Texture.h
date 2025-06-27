#ifndef TEXTURE_H
#define TEXTURE_H

#include "Util.h"

class Texture {
public:
	enum class TextureType : int8
	{
		unknown = -1,
		diffuse,
		specular,
	};

private:
	uint m_rendererID;

	int m_width, m_height;
	int m_bytesPerPixel;

	std::string m_filepath;

	TextureType m_texType;

public:
	Texture();
	explicit Texture(const std::string& path, TextureType texType = TextureType::diffuse, bool flipVertically = false, bool mip = true);

	~Texture();

	void Init(const std::string& path, TextureType texType = TextureType::diffuse, bool flipVertically = false, bool mip = true);

	void Bind(uint slot = 0) const;
	void Unbind() const;

	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }
	TextureType GetType() const { return m_texType; }
	std::string GetFilePath() const { return m_filepath; }

	static std::string ConvertTextureTypeToString(TextureType texType);
};
#endif //TEXTURE_H
