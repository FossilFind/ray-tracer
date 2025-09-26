#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "shader.h"
#include "camera.h"

#define PI 3.14159265359

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

#define COMPUTE_PATH "src/shaders/pathtrace.glsl"
#define VERTEX_PATH "src/shaders/vertex.glsl"
#define FRAGMENT_PATH "src/shaders/fragment.glsl"

#define LOOK_SPEED 0.01
#define ZOOM_SPEED 0.05
#define MOVE_SPEED 0.1

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
	glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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

void callbackCursorPos(GLFWwindow *window, double xPos, double yPos)
{
	struct Camera *camera = (struct Camera *) glfwGetWindowUserPointer(window);

	camera->yaw += xPos * LOOK_SPEED;
	camera->pitch += yPos * LOOK_SPEED;

	if(camera->pitch > PI / 2)
		camera->pitch = PI / 2;

	if(camera->pitch < -PI / 2)
		camera->pitch = -PI / 2;

	glfwSetCursorPos(window, 0, 0);
}

void callbackScroll(GLFWwindow *window, double xOffset, double yOffset)
{
	struct Camera *camera = (struct Camera *) glfwGetWindowUserPointer(window);

	camera->fov += yOffset * ZOOM_SPEED;
}

void setCallbacks(GLFWwindow *window)
{
	glfwSetFramebufferSizeCallback(window, callbackFrambufferSize);
	glfwSetCursorPosCallback(window, callbackCursorPos);
	glfwSetScrollCallback(window, callbackScroll);
}

//----Callbacks----

void processInput(GLFWwindow *window, struct Camera * const camera)
{
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	double fowardMovement = 0;

	if(glfwGetKey(window, GLFW_KEY_W))
		fowardMovement += MOVE_SPEED;
	
	if(glfwGetKey(window, GLFW_KEY_S))
		fowardMovement -= MOVE_SPEED;

	double camDirX = -cos(-camera->pitch) * cos(-camera->yaw);
	double camDirY = -cos(-camera->pitch) * sin(-camera->yaw);
	double camDirLen = sqrt(camDirX * camDirX + camDirY * camDirY);
	camDirX /= camDirLen;
	camDirY /= camDirLen;

	double deltaX = camDirX * fowardMovement;
	double deltaY = camDirY * fowardMovement;

	double sideMovement = 0;

	if(glfwGetKey(window, GLFW_KEY_A))
		sideMovement += MOVE_SPEED;

	if(glfwGetKey(window, GLFW_KEY_D))
		sideMovement -= MOVE_SPEED;
	
	double sideDirX = -camDirY;
	double sideDirY = camDirX;

	deltaX += sideDirX * sideMovement;
	deltaY += sideDirY * sideMovement;

	camera->posX += deltaX;
	camera->posY += deltaY;
}

void render(GLFWwindow *window, const uint32_t computeProgram, const uint32_t renderProgram, uint32_t vao, const struct Camera cam)
{
	glUseProgram(computeProgram);

	glUniform1f(glGetUniformLocation(computeProgram, "camFOV"), cam.fov);
	glUniform1f(glGetUniformLocation(computeProgram, "camYaw"), cam.yaw);
	glUniform1f(glGetUniformLocation(computeProgram, "camPitch"), cam.pitch);
	glUniform3f(glGetUniformLocation(computeProgram, "camPos"), cam.posX, cam.posY, cam.posZ);

	glDispatchCompute(cam.width, cam.height, 1);

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

	// Set shader uniforms
	glUseProgram(computeProgram);

	glUniform1f(glGetUniformLocation(computeProgram, "camFOV"), PI / 2);
	glUniform3f(glGetUniformLocation(computeProgram, "camDir"), -1.0f, 0.0f, 0.0f);

	glUniform3f(glGetUniformLocation(computeProgram, "triPoint0"), 1.0f, -0.5f, 0.5f);
	glUniform3f(glGetUniformLocation(computeProgram, "triPoint1"), 1.0f, -0.5f, -0.5f);
	glUniform3f(glGetUniformLocation(computeProgram, "triPoint2"), 1.0f, 0.5f, -0.5f);

	struct Camera cam = {WINDOW_WIDTH, WINDOW_HEIGHT, PI / 2, 0, 0, 0, 0, 0};
	glfwSetWindowUserPointer(window, &cam);

	// Render loop
	while(!glfwWindowShouldClose(window))
	{
		processInput(window, &cam);

		render(window, computeProgram, renderProgram, vao, cam);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Clean up
	glfwTerminate();

	return 0;
}