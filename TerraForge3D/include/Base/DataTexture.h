#pragma once

#include <string>
#include <glad/glad.h>


class DataTexture
{
public:
	DataTexture()
	{
		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glGenerateMipmap(GL_TEXTURE_2D);
	}

	virtual ~DataTexture()
	{
		glDeleteTextures(1, &m_RendererID);
		if (pData) delete[] pData;
	}

	inline void Resize(int resolution)
	{
		if (pData) delete[] pData;
		this->size = resolution;
		pData = new float[size * size * 4];
		memset(pData, 0, sizeof(float) * size * size * 4);
		this->UploadToGPU();
	}

	inline void SetData(float* data_, size_t size_, size_t offset_)
	{
		memcpy(pData + offset_, data_, size_);
	}

	inline void Bind(int slot)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
	}

	inline void UploadToGPU()
	{
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, size, size, 0, GL_RGBA, GL_FLOAT, pData);
	}

	inline void SetPixelI(int x, int y, float r, float g, float b, float a)
	{
		float* pixel = &pData[(y * size + x) * 4];
		pixel[0] = r; pixel[1] = g; pixel[2] = b; pixel[3] = a;
	}

	inline void SetPixelF(float tx, float ty, float r, float g, float b, float a)
	{
		int x = (int)(tx * (size - 1)); int y = (int)(ty * (size - 1));
		x = std::clamp(x, 0, size - 1); y = std::clamp(y, 0, size - 1);
		float* pixel = &pData[(y * size + x) * 4];
		pixel[0] = r; pixel[1] = g; pixel[2] = b; pixel[3] = a;
	}

	inline void GetPixelI(int x, int y, float* r = nullptr, float* g = nullptr, float* b = nullptr, float* a = nullptr)
	{
		float* pixel = &pData[(y * size + x) * 4];
		if (r) *r = pixel[0];
		if (g) *g = pixel[1];
		if (b) *b = pixel[2];
		if (a) *a = pixel[3];
	}

	inline void GetPixelF(float tx, float ty, float* r = nullptr, float* g = nullptr, float* b = nullptr, float* a = nullptr)
	{
		int x = (int)(tx * (size - 1)); int y = (int)(ty * (size - 1));
		x = std::clamp(x, 0, size - 1); y = std::clamp(y, 0, size - 1);
		float* pixel = &pData[(y * size + x) * 4];
		if (r) *r = pixel[0];
		if (g) *g = pixel[1];
		if (b) *b = pixel[2];
		if (a) *a = pixel[3];
	}

	inline float* GetDataPtr() { return pData; }

	inline size_t GetSize() { return sizeof(float) * 4 * size * size; }

private:

	float* pData = nullptr;
	int size = 0;
	GLuint m_RendererID = 0;
};

