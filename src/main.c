#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>

#include "cartridge.h"
#include "gameboy.h"
#include "display.h"
#include "input.h"

static Cartridge cartridge;
static Gameboy gameboy;
static GLFWwindow *window;

static uint8_t framebuffer[23040][3];
const static uint8_t pallet[4][3] = {
	{196, 207, 161},
	{139, 149, 109},
	{107, 115, 83},
	{65, 65, 65}
};

void renderFunction(int x, int y, int color)
{
	framebuffer[(y * 160) + x][0] = pallet[color][0];
	framebuffer[(y * 160) + x][1] = pallet[color][1];
	framebuffer[(y * 160) + x][2] = pallet[color][2];
}

void drawFunction()
{
	int width;
	int height;
	glfwGetFramebufferSize(window, &width, &height);

	/**
	 * Clear window.
	 */
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, height, 0, 0, 1);
	
	/**
	 * Generate texture from framebuffer.
	 */
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 160, 144, 0, GL_RGB, GL_UNSIGNED_BYTE, framebuffer);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/**
	 * Define and draw texture coordinates.
	 */
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_QUADS);

	glTexCoord2f(0, 0); glVertex2f(0, 0); 
	glTexCoord2f(1, 0); glVertex2f(width, 0); 
	glTexCoord2f(1, 1); glVertex2f(width, height); 
	glTexCoord2f(0, 1); glVertex2f(0, height); 

	glEnd();
	glDisable(GL_TEXTURE_2D);
	glDeleteTextures(1, &texture);

	glfwSwapBuffers(window);
}

void handleInput(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GL_TRUE);
			break;

		case GLFW_KEY_UP:
			if (action == GLFW_PRESS) {
				inputTrigger(&gameboy.joypad, &gameboy.mmu, BUTTON_UP);
			} else if (action == GLFW_RELEASE) {
				inputRelease(&gameboy.joypad, BUTTON_UP);
			}
			break;

		case GLFW_KEY_DOWN:
			if (action == GLFW_PRESS) {
				inputTrigger(&gameboy.joypad, &gameboy.mmu, BUTTON_DOWN);
			} else if (action == GLFW_RELEASE) {
				inputRelease(&gameboy.joypad, BUTTON_DOWN);
			}
			break;

		case GLFW_KEY_LEFT:
			if (action == GLFW_PRESS) {
				inputTrigger(&gameboy.joypad, &gameboy.mmu, BUTTON_LEFT);
			} else if (action == GLFW_RELEASE) {
				inputRelease(&gameboy.joypad, BUTTON_LEFT);
			}
			break;

		case GLFW_KEY_RIGHT:
			if (action == GLFW_PRESS) {
				inputTrigger(&gameboy.joypad, &gameboy.mmu, BUTTON_RIGHT);
			} else if (action == GLFW_RELEASE) {
				inputRelease(&gameboy.joypad, BUTTON_RIGHT);
			}
			break;

		case GLFW_KEY_ENTER:
			if (action == GLFW_PRESS) {
				inputTrigger(&gameboy.joypad, &gameboy.mmu, BUTTON_START);
			} else if (action == GLFW_RELEASE) {
				inputRelease(&gameboy.joypad, BUTTON_START);
			}
			break;

		case GLFW_KEY_SPACE:
			if (action == GLFW_PRESS) {
				inputTrigger(&gameboy.joypad, &gameboy.mmu, BUTTON_SELECT);
			} else if (action == GLFW_RELEASE) {
				inputRelease(&gameboy.joypad, BUTTON_SELECT);
			}
			break;

		case GLFW_KEY_Z:
			if (action == GLFW_PRESS) {
				inputTrigger(&gameboy.joypad, &gameboy.mmu, BUTTON_B);
			} else if (action == GLFW_RELEASE) {
				inputRelease(&gameboy.joypad, BUTTON_B);
			}
			break;

		case GLFW_KEY_X:
			if (action == GLFW_PRESS) {
				inputTrigger(&gameboy.joypad, &gameboy.mmu, BUTTON_A);
			} else if (action == GLFW_RELEASE) {
				inputRelease(&gameboy.joypad, BUTTON_A);
			}
			break;
	}
}

int main(int argc, const char* argv[])
{
	int error;
	char gamePath[256];

	/**
	 * Load the gameboy cartridge
	 */
	if (argc < 2) {
		printf("ERROR: Cartridge path not provided.\n");
		getchar();
		return -1;
	}

	strncpy(gamePath, argv[1], sizeof(gamePath) - 1);
	gamePath[sizeof(gamePath) - 1] = '\0';

	printf("EVENT: Loading cartridge...\n");

	error = cartridgeInit(&cartridge, gamePath);

	if (error == -1) {
		printf("ERROR: Could not find cartridge file.\n");
		getchar();
		return -1;
	}
	
	if (error == -2) {
		printf("ERROR: Could not read cartridge file.\n");
		getchar();
		return -1;
	}

	/**
	 * Initialize and run the gameboy
	 */
	printf("EVENT: Initializing gameboy...\n");

	gameboyInit(&gameboy, &cartridge);

	/**
	 * Initialize GLFW
	 */
	if (!glfwInit()) {
		printf("ERROR: Failed to initialize GLFW.\n");
		getchar();
		return -1;
	}

	window = glfwCreateWindow(160, 144, "FoxxGB", NULL, NULL);

	if (!window) {
		printf("ERROR: Failed to create application window.\n");
		glfwTerminate();
		getchar();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	/**
	 * Bind various callbacks
	 */
	displaySetRenderCallback(&gameboy.display, renderFunction);
	displaySetDrawCallback(&gameboy.display, drawFunction);
	glfwSetKeyCallback(window, handleInput);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		gameboyStep(&gameboy);
	}

	/**
	 * Terminate
	 */
	glfwTerminate();

	printf("EVENT: Cleaning up memory...\n");

	return 0;
}
