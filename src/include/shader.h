#pragma once

#include <inttypes.h>

int32_t createComputeProgram(const char *shaderPath, uint32_t *programID, char **compileError);
int32_t createRenderProgram(const char *vertexPath, const char *fragmentPath, uint32_t *programID, char **compileError);