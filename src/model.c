#include "model.h"

#include <glad/gl.h>

struct Point
{
	float x, y, z, w;
};

int32_t generateSSBO(const int32_t bindingPoint, const struct Point points[], const int32_t numTriangles)
{
	uint32_t ssbo;
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, numTriangles * sizeof(struct Point), points, GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, ssbo);
}

int32_t generateModel()
{
	struct Point v0[] =
	{
		{0.5, -0.5, 0.5, 0.0},
		{0.5, -0.5, 0.5, 0.0},
		{0.5, 0.5, 0.5, 0.0},
		{0.5, 0.5, 0.5, 0.0},
		{-0.5, 0.5, 0.5, 0.0},
		{-0.5, 0.5, 0.5, 0.0},
		{-0.5, -0.5, 0.5, 0.0},
		{-0.5, -0.5, 0.5, 0.0},
		{-0.5, -0.5, 0.5, 0.0},
		{-0.5, -0.5, 0.5, 0.0},
		{0.5, -0.5, -0.5, 0.0},
		{0.5, -0.5, -0.5, 0.0}
	};
	generateSSBO(1, v0, sizeof(v0) / sizeof(v0[0]));

	struct Point v1[] =
	{
		{0.5, -0.5, -0.5, 0.0},
		{0.5, 0.5, -0.5, 0.0},
		{0.5, 0.5, -0.5, 0.0},
		{-0.5, 0.5, -0.5, 0.0},
		{-0.5, 0.5, -0.5, 0.0},
		{-0.5, -0.5, -0.5, 0.0},
		{-0.5, -0.5, -0.5, 0.0},
		{0.5, -0.5, -0.5, 0.0},
		{0.5, -0.5, 0.5, 0.0},
		{0.5, 0.5, 0.5, 0.0},
		{-0.5, -0.5, -0.5, 0.0},
		{-0.5, 0.5, -0.5, 0.0}
	};
	generateSSBO(2, v1, sizeof(v1) / sizeof(v1[0]));

	struct Point v2[] =
	{
		{0.5, 0.5, -0.5, 0.0},
		{0.5, 0.5, 0.5, 0.0},
		{-0.5, 0.5, -0.5, 0.0},
		{-0.5, 0.5, 0.5, 0.0},
		{-0.5, -0.5, -0.5, 0.0},
		{-0.5, -0.5, 0.5, 0.0},
		{0.5, -0.5, -0.5, 0.0},
		{0.5, -0.5, 0.5, 0.0},
		{0.5, 0.5, 0.5, 0.0},
		{-0.5, 0.5, 0.5, 0.0},
		{-0.5, 0.5, -0.5, 0.0},
		{0.5, 0.5, -0.5, 0.0}
	};
	generateSSBO(3, v2, sizeof(v2) / sizeof(v2[0]));
}