#ifndef TEXT_RENDER_H
#define TEXT_RENDER_H
#include "../BaseRender.h"
#include "../Shader.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <deque>

struct FontHeader
{
	uint32_t fourcc;
	uint32_t version;
	uint32_t start_ptr;
	uint32_t validate_chars;
	uint32_t non_empty_chars;
	uint32_t char_size;

	int16_t base;
	int16_t scale;
};

struct FontInfo
{
	int16_t top;
	int16_t left;
	uint16_t width;
	uint16_t height;
};

struct LRUCharInfo
{
	double top, left, width, height;
	uint64_t tick;
};


class TextRender: public BaseRender
{
public:
	TextRender(const std::string filename = "resource/Microsoft_YaHei.kfont");
	~TextRender();
	void Render();
	void AddText2D(std::wstring text, int base_x, int base_y, double scale = 1.0);
private:
	uint32_t makeFourCC(unsigned char ch0, unsigned char ch1, unsigned char ch2, unsigned char ch3)
	{
		return (ch0 << 0) + (ch1 << 8) + (ch2 << 16) + (ch3 << 24);
	}

	void GetLZMADistanceData(uint8_t* p, uint32_t& size, int32_t index)
	{
		if (p != nullptr)
		{
			int offset = encodeCharLzmaStart + (index + 1) * sizeof(uint64_t) + fontBitmapAddrVec[index];
			fontStream.seekg(offset, std::ios_base::beg);
			fontStream.read(reinterpret_cast<char*>(p), size);
		}
	}

	bool loadFontLibrary(std::string filename);

	Shader *ourShader;

	std::ifstream fontStream;
	FontHeader header;
	int64_t encodeCharLzmaStart;
	std::vector<FontInfo> charInfoVec;
	std::vector<size_t> fontBitmapAddrVec;
	std::unordered_map<int32_t, std::pair<int32_t, uint32_t>> charIndexAdvance;

	GLuint lruTexture;
	std::deque<int> freeCharIndex;
	std::vector<uint8_t> lruTextureData;
	std::unordered_map<wchar_t, LRUCharInfo> lruCharMap;

	std::vector<GLuint> VAOVec;
	std::vector<GLuint> VBOVec;
	std::vector<std::vector<int>> countVec;
	std::vector<std::vector<int>> indicesVec;

	double pixelPerWidth, pixelPerHeight;
	unsigned int textureSize, perLineCharNum;
	uint64_t tick_;

    std::string fontFileName;
};
#endif // TEXT_RENDER_H