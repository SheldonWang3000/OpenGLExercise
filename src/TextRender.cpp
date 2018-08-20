#include "TextRender.h"
#include "LzmaLib.h"
TextRender::TextRender(const std::string filename /*= "resource/Microsoft_YaHei.kfont"*/)
{
	loadFontLibrary(filename);

	tick_ = 0;
	textureSize = 2048;
	perLineCharNum = textureSize / header.char_size;
	lruTextureData.resize(textureSize * textureSize);
	for (int i = 0; i < perLineCharNum * perLineCharNum; ++i) freeCharIndex.push_back(i);

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, &(viewport[0]));
	double bouns = 5;
	pixelPerWidth = (2.0 / viewport[2]) / bouns;
	pixelPerHeight = (2.0 / viewport[3]) / bouns;

	glGenTextures(1, &lruTexture);
	glBindTexture(GL_TEXTURE_2D, lruTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, textureSize, textureSize, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);

	ourShader = new Shader("./Shader/font.vs", "./Shader/font.fs");

	AddText2D(L"Hello", 200, 200);
}

TextRender::~TextRender()
{
	delete ourShader;
	glDeleteVertexArrays(VAOVec.size(), &(VAOVec.data()[0]));
	glDeleteBuffers(VBOVec.size(), &(VBOVec.data()[0]));
}

void TextRender::Render()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, lruTexture);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	ourShader->use();

	float base = header.base / 32768.0f * 32 + 1;
	float scale = (header.scale / 32768.0f + 1.0f) * 32;

	glUniform1i(glGetUniformLocation(ourShader->ID, "texture1"), 0);
	glUniform1f(glGetUniformLocation(ourShader->ID, "base"), base);
	glUniform1f(glGetUniformLocation(ourShader->ID, "scale"), scale);

	for (int i = 0; i < VAOVec.size(); ++i)
	{
		glBindVertexArray(VAOVec[i]);
		glMultiDrawArrays(GL_TRIANGLE_STRIP, indicesVec[i].data(), countVec[i].data(), countVec[i].size());
	}
}

void TextRender::AddText2D(std::wstring text, int base_x, int base_y, double scale /*= 1.0*/)
{
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> texCoords;
	std::vector<int> count;
	std::vector<int> indices;
	GLuint verticesVBO, texVBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &verticesVBO);
	glGenBuffers(1, &texVBO);

	int x = base_x;
	int y = base_y;

	uint32_t char_size = header.char_size;
	std::vector<uint8_t> decoded(char_size * char_size);
	++tick_;
	for (auto ch : text)
	{
		if (ch != L'\n')
		{
			auto itr = charIndexAdvance.find(ch);
			if (itr == charIndexAdvance.end())
			{
				x += char_size;
				continue;
			}
			if (itr->second.first == -1)
			{
				x += itr->second.second & 0xFFFF;
				y += itr->second.second >> 16;
				continue;
			}

			auto offset = itr->second.first;
			FontInfo info = charInfoVec[offset];
			auto lruCharMapItr = lruCharMap.find(ch);
			LRUCharInfo lruCharInfo;
			if (lruCharMapItr != lruCharMap.end())
			{
				// has in lru
				lruCharMapItr->second.tick = tick_;
				lruCharInfo = lruCharMapItr->second;
			}
			else
			{
				// has not used
				uint32_t size;
				size = static_cast<uint32_t>(fontBitmapAddrVec[offset + 1] - fontBitmapAddrVec[offset]);
				std::vector<uint8_t> in_data(size);
				GetLZMADistanceData(&in_data[0], size, offset);

				size_t s_out_len = decoded.size();
				size_t s_src_len = in_data.size() - LZMA_PROPS_SIZE;
				LzmaUncompress(static_cast<unsigned char*>(&decoded[0]), &s_out_len, &in_data[LZMA_PROPS_SIZE], &s_src_len,
					&in_data[0], LZMA_PROPS_SIZE);


				if (freeCharIndex.size() == 0)
				{
					int minTick = UINT64_MAX;
					for (auto itr = lruCharMap.begin(); itr != lruCharMap.end(); ++itr)
					{
						if (itr->second.tick < minTick)
						{
							minTick = itr->second.tick;
						}
					}
					for (auto itr = lruCharMap.begin(); itr != lruCharMap.end(); ++itr)
					{
						if (itr->second.tick == minTick)
						{
							int index = static_cast<int>(itr->second.left * textureSize) / char_size;
							index += static_cast<int>(itr->second.top * textureSize) / char_size * perLineCharNum;
							freeCharIndex.push_back(index);
							itr = lruCharMap.erase(itr);
						}
					}
				}

				int index = freeCharIndex.front();
				freeCharIndex.pop_front();
				int texY = index / perLineCharNum;
				int texX = index - texY * perLineCharNum;
				texX *= char_size;
				texY *= char_size;

				for (int i = 0; i < char_size; ++i)
				{
					for (int j = 0; j < char_size; ++j)
					{
						int src = i * char_size + j;
						int target = (i + texY) * textureSize + j + texX;
						lruTextureData[target] = decoded[src];
					}
				}

				lruCharInfo.left = static_cast<double>(texX) / textureSize;
				lruCharInfo.top = static_cast<double>(texY) / textureSize;
				lruCharInfo.width = static_cast<double>(info.width) / textureSize;
				lruCharInfo.height = static_cast<double>(info.height) / textureSize;
				lruCharInfo.tick = tick_;

				lruCharMap.insert(std::make_pair(ch, lruCharInfo));
			}

			vertices.emplace_back((x + info.left) * pixelPerWidth * 2 - 1, -((y + info.top + info.height) * pixelPerHeight * 2 - 1), 0.0f);
			vertices.emplace_back((x + info.left + info.width) * pixelPerWidth * 2 - 1, -((y + info.top + info.height) * pixelPerHeight * 2 - 1), 0.0f);
			vertices.emplace_back((x + info.left) * pixelPerWidth * 2 - 1, -((y + info.top) * pixelPerHeight * 2 - 1), 0.0f);
			vertices.emplace_back((x + info.left + info.width) * pixelPerWidth * 2 - 1, -((y + info.top) * pixelPerHeight * 2 - 1), 0.0f);


			count.push_back(4);
			x += (itr->second.second & 0xFFFF);
			y += (itr->second.second >> 16);

			texCoords.emplace_back(lruCharInfo.left, lruCharInfo.top + lruCharInfo.height);
			texCoords.emplace_back(lruCharInfo.left + lruCharInfo.width, lruCharInfo.top + lruCharInfo.height);
			texCoords.emplace_back(lruCharInfo.left, lruCharInfo.top);
			texCoords.emplace_back(lruCharInfo.left + lruCharInfo.width, lruCharInfo.top);
		}
		else
		{
			x = base_x;
			y += char_size;
		}
	}

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, verticesVBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, texVBO);
	glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(glm::vec2), texCoords.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);


	indices.push_back(0);
	for (int i = 0; i < count.size() - 1; ++i)
	{
		indices.push_back(indices.back() + 4);
	}
	VAOVec.push_back(VAO);
	VBOVec.push_back(verticesVBO);
	VBOVec.push_back(texVBO);
	countVec.push_back(count);
	indicesVec.push_back(indices);
	glTextureSubImage2D(lruTexture, 0, 0, 0, textureSize, textureSize, GL_RED, GL_UNSIGNED_BYTE, static_cast<unsigned char*>(&lruTextureData[0]));
	glGenerateMipmap(GL_TEXTURE_2D);

}

bool TextRender::loadFontLibrary(std::string filename)
{
	fontStream.open(filename, std::ios::binary);
	if (!fontStream.is_open())
	{
		return false;
	}

	fontStream.read(reinterpret_cast<char*>(&header), sizeof(header));

	// verify the fourcc
	if (makeFourCC('K', 'F', 'N', 'T') == header.fourcc)
	{
		fontStream.seekg(header.start_ptr, std::ios_base::beg);

		std::vector<std::pair<int32_t, int32_t>> temp_char_index(header.non_empty_chars);
		fontStream.read(reinterpret_cast<char*>(&temp_char_index[0]), temp_char_index.size() * sizeof(temp_char_index[0]));

		std::vector<std::pair<int32_t, uint32_t>> temp_char_advance(header.validate_chars);
		fontStream.read(reinterpret_cast<char*>(&temp_char_advance[0]), temp_char_advance.size() * sizeof(temp_char_advance[0]));

		for (auto& ci : temp_char_index)
		{
			charIndexAdvance.emplace(ci.first, std::make_pair(ci.second, 0));
		}

		for (auto& ca : temp_char_advance)
		{
			auto iter = charIndexAdvance.find(ca.first);
			if (iter != charIndexAdvance.end())
			{
				iter->second.second = ca.second;
			}
			else
			{
				charIndexAdvance[ca.first] = std::make_pair(-1, ca.second);
			}
		}

		charInfoVec.resize(header.non_empty_chars);
		fontStream.read(reinterpret_cast<char*>(&charInfoVec[0]), charInfoVec.size() * sizeof(charInfoVec[0]));

		fontBitmapAddrVec.resize(header.non_empty_chars + 1);
		encodeCharLzmaStart = fontStream.tellg();
		size_t distances_lzma_size = 0;

		for (uint32_t i = 0; i < header.non_empty_chars; ++i)
		{
			fontBitmapAddrVec[i] = distances_lzma_size;

			uint64_t len;
			fontStream.read(reinterpret_cast<char*>(&len), sizeof(len));
			distances_lzma_size += static_cast<size_t>(len);

			fontStream.seekg(len, std::ios_base::cur);
		}

		fontBitmapAddrVec[header.non_empty_chars] = distances_lzma_size;
	}

	return true;
}