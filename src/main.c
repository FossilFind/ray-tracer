#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "shader.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define COMPUTE_PATH "src/shaders/pathtrace.glsl"
#define VERTEX_PATH "src/shaders/vertex.glsl"
#define FRAGMENT_PATH "src/shaders/fragment.glsl"

int32_t glfwSetup(GLFWwindow **window)
{
	if(!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);

	// Create window
	*window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Ray Tracer", NULL, NULL);
	if(*window == NULL)
	{
		glfwTerminate();
		return -2;
	}
	glfwMakeContextCurrent(*window);
}

int32_t glSetup()
{
	if(gladLoadGL(glfwGetProcAddress) < 0)
		return -1;

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}

void createScreenTexture(const uint32_t width, const uint32_t height)
{
	uint32_t texture;

	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

	glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
}

void createRenderQuad(uint32_t *vao)
{
	float quadVertices[] =
	{
		// Positions        // Texture Coords
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};

	// Setup plane VAO
	uint32_t vbo;
	
	glGenVertexArrays(1, vao);
	glGenBuffers(1, &vbo);
	glBindVertexArray(*vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
}

//----Callbacks----

void callbackFrambufferSize(GLFWwindow *window, int32_t width, int32_t height)
{
	glViewport(0, 0, width, height);
	createScreenTexture(width, height);
}

void setCallbacks(GLFWwindow *window)
{
	glfwSetFramebufferSizeCallback(window, callbackFrambufferSize);
}

//----Callbacks----

void processInput(GLFWwindow *window)
{
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

void render(GLFWwindow *window, const uint32_t computeProgram, const uint32_t renderProgram, uint32_t vao)
{
	glUseProgram(computeProgram);
	glDispatchCompute(WINDOW_WIDTH, WINDOW_HEIGHT, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	glClear(GL_COLOR_BUFFER_BIT);
	
	glUseProgram(renderProgram);

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

/**
 * Error 1: GLFW setup error
 * Error 2: GL setup error
 * Error 3: Compute shader creation error
 * Error 4: Render shader creation error
 */
int32_t main()
{
	int32_t error;

	// Setup
	GLFWwindow *window;
	if((error = glfwSetup(&window)) < 0)
	{
		switch(error)
		{
		case -1:
			printf("Failed to initialize GLFW\n");
			break;
		case -2:
			printf("Failed to create window\n");
			break;
		}

		return -1;
	}

	if(glSetup() < 0)
	{
		printf("Failed to initialize GLAD\n");
		return -2;
	}

	setCallbacks(window);

	// Setup shaders
	char *compErr;

	uint32_t computeProgram;
	if((error = createComputeProgram(COMPUTE_PATH, &computeProgram, &compErr)) < 0)
	{
		switch(error)
		{
		case -1:
			printf("Compute shader file not found\n");
			break;
		case -2:
			printf("Failed to read compute shader file\n");
			break;
		case -3:
			printf("Compute program compilation error:\n%s\n", compErr);
			break;
		}

		return -3;
	}
	free(compErr);

	uint32_t renderProgram;
	if((error = createRenderProgram(VERTEX_PATH, FRAGMENT_PATH, &renderProgram, &compErr)) < 0)
	{
		switch(error)
		{
		case -1:
			printf("Vertex shader file not found\n");
			break;
		case -2:
			printf("Failed to read vertex shader file\n");
			break;
		case -3:
			printf("Fragment shader file not found\n");
			break;
		case -4:
			printf("Failed to read fragment shader file\n");
			break;
		case -5:
			printf("Render program compilation err:\n%s\n", compErr);
			break;
		}

		return -4;
	}
	free(compErr);

	createScreenTexture(WINDOW_WIDTH, WINDOW_HEIGHT);

	uint32_t vao;
	createRenderQuad(&vao);

	// Render loop
	while(!glfwWindowShouldClose(window))
	{
		processInput(window);

		render(window, computeProgram, renderProgram, vao);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Clean up
	glfwTerminate();

	return 0;
}