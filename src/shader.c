#include "shader.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#define INFO_LOG_LENGTH 512

/**
 * Error 1: File not found
 * Error 2: Read error
 */
int32_t readFile(const char *fileName, char **buf, size_t *bufSize)
{
	FILE *fPtr = fopen(fileName, "r");

	if(fPtr == NULL)
		return -1;

	// Go to end of file
	if(fseek(fPtr, 0L, SEEK_END) != 0)
	{
		fclose(fPtr);
		return -2;
	}

	// Read pointer pos to find file length
	*bufSize = ftell(fPtr); 
	if(*bufSize == -1)
	{
		fclose(fPtr);
		return -2;
	}

	*buf = malloc(sizeof(char) * ((*bufSize) + 1));

	// Return to front of file
	if(fseek(fPtr, 0L, SEEK_SET) != 0)
	{
		fclose(fPtr);
		free(*buf);
		return -2;
	}

	*bufSize = fread(*buf, sizeof(char), *bufSize, fPtr);

	if(ferror(fPtr) != 0)
	{
		fclose(fPtr);
		free(*buf);
		return -2;
	}

	(*buf)[(*bufSize)++] = '\0';

	fclose(fPtr);

	return 0;
}

/**
 * Error 1-2: File error
 * Error 3: Compilation error
 */
int32_t createComputeProgram(const char *shaderPath, uint32_t *programID, char **compileError)
{
	int32_t error;
	if(compileError != NULL)
	{
		*compileError = malloc(sizeof(char) * INFO_LOG_LENGTH * 2);
		**compileError = '\0';
	}

	// Read file
	char *buf;
	size_t bufSize;
	if((error = readFile(shaderPath, &buf, &bufSize)) < 0)
		return error;

	// Compile shader
	uint32_t shader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(shader, 1, (const char * const *) &buf, NULL);
	glCompileShader(shader);

	free(buf);

	// Check for compilation errors
	int32_t compileSuccess;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileSuccess);
	if(!compileSuccess)
	{
		if(compileError != NULL)
		{
			char infoLog[INFO_LOG_LENGTH];
			glGetShaderInfoLog(shader, INFO_LOG_LENGTH, NULL, infoLog);
			strcat(*compileError, infoLog);
		}

		return -3;
	}

	// Link program
	*programID = glCreateProgram();
	glAttachShader(*programID, shader);
	glLinkProgram(*programID);

	glDeleteShader(shader);

	// Check for linking errors
	int32_t linkSuccess;
	glGetProgramiv(*programID, GL_LINK_STATUS, &linkSuccess);
	if(!linkSuccess)
	{
		if(compileError != NULL)
		{
			char infoLog[INFO_LOG_LENGTH];
			glGetProgramInfoLog(*programID, INFO_LOG_LENGTH, NULL, infoLog);
			strcat(*compileError, infoLog);
		}

		return -3;
	}

	return 0;
}

/**
 * Error 1-2: Vertex file error
 * Error 3-4: Fragment file error
 * Error 5: Compilation error
 */
int32_t createRenderProgram(const char *vertexPath, const char *fragmentPath, uint32_t *programID, char **compileError)
{
	int32_t error;
	if(compileError != NULL)
	{
		*compileError = malloc(sizeof(char) * INFO_LOG_LENGTH * 3);
		**compileError = '\0';
	}

	// Read files
	char *vertBuf;
	size_t vertBufSize;
	if((error = readFile(vertexPath, &vertBuf, &vertBufSize)) < 0)
		return error;

	char *fragBuf;
	size_t fragBufSize;
	if((error = readFile(fragmentPath, &fragBuf, &fragBufSize)) < 0)
		return error - 2;

	// Compile vertex
	uint32_t vertShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertShader, 1, (const char * const *) &vertBuf, NULL);
	glCompileShader(vertShader);

	free(vertBuf);

	// Check for compilation errors
	int32_t vertCompileSuccess;
	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &vertCompileSuccess);
	if(!vertCompileSuccess && compileError != NULL)
	{
		char infoLog[INFO_LOG_LENGTH];
		glGetShaderInfoLog(vertShader, INFO_LOG_LENGTH, NULL, infoLog);
		strcat(*compileError, infoLog);
	}

	// Compile fragment
	uint32_t fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, (const char * const *) &fragBuf, NULL);
	glCompileShader(fragShader);

	free(fragBuf);

	// Check for compilation errors
	int32_t fragCompileSuccess;
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &fragCompileSuccess);
	if(!fragCompileSuccess && compileError != NULL)
	{
		char infoLog[INFO_LOG_LENGTH];
		glGetShaderInfoLog(fragShader, INFO_LOG_LENGTH, NULL, infoLog);
		strcat(*compileError, infoLog);
	}

	if(!vertCompileSuccess || !fragCompileSuccess)
	{
		return -5;
	}

	// Link program
	*programID = glCreateProgram();
	glAttachShader(*programID, vertShader);
	glAttachShader(*programID, fragShader);
	glLinkProgram(*programID);
	
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	// Check for linking errors
	int32_t linkSuccess;
	glGetProgramiv(*programID, GL_LINK_STATUS, &linkSuccess);
	if(!linkSuccess && compileError != NULL)
	{
		if(compileError != NULL)
		{
			char infoLog[INFO_LOG_LENGTH];
			glGetProgramInfoLog(*programID, INFO_LOG_LENGTH, NULL, infoLog);
			strcat(*compileError, infoLog);
		}

		return -5;
	}

	return 0;
}