#define GLEW_STATIC
#include <GL\glew.h>

#include <GLFW/glfw3.h>

#include "TransparencyScene.h"
#include "IScene.h"
#include <string>
#include "Camera.h"
#include <sstream>

using namespace std; 

#define WIN_WIDTH 1024
#define WIN_HEIGHT 768

Camera camera(glm::vec3(-22.0f, 10.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f), 1.25f, 3.75f);
GLfloat lastX = WIN_WIDTH / 2.0;
GLfloat lastY = WIN_HEIGHT / 2.0;
bool    keys[1024];

IScene *scene;
GLFWwindow * window;
std::string title;

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key >= 0 && key < 1024) {
		if (action == GLFW_PRESS) {
			keys[key] = true;
		}
		else if (action == GLFW_RELEASE) {
			keys[key] = false;
		}
	}
	if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
		scene->animate(!(scene->animating()));
}

void do_movement() {
	float speed = 1.0f;
	if (keys[GLFW_KEY_LEFT_SHIFT]) {
		speed = 10.0f;
	}
	if (keys[GLFW_KEY_Y]) {
		((TransparencyScene*)scene)->removeNode();
	}
	if (keys[GLFW_KEY_U]) {
		((TransparencyScene*)scene)->addNode();
	}
	// Camera controls
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime * speed);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime * speed);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime * speed);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime * speed);
	if (keys[GLFW_KEY_Z])
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if (keys[GLFW_KEY_X])
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

bool firstMouse = true;
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

double calcFPS(double theTimeInterval = 1.0)
{
	// Static values which only get initialised the first time the function runs
	static double t0Value = glfwGetTime(); // Set the initial time to now
	static int    fpsFrameCount = 0;             // Set the initial FPS frame count to 0
	static double fps = 0.0;           // Set the initial FPS value to 0.0

									   // Get the current time in seconds since the program started (non-static, so executed every time)
	double currentTime = glfwGetTime();

	// Ensure the time interval between FPS checks is sane (low cap = 0.1s, high-cap = 10.0s)
	// Negative numbers are invalid, 10 fps checks per second at most, 1 every 10 secs at least.
	if (theTimeInterval < 0.1)
	{
		theTimeInterval = 0.1;
	}
	if (theTimeInterval > 10.0)
	{
		theTimeInterval = 10.0;
	}

	// Calculate and display the FPS every specified time interval
	if ((currentTime - t0Value) > theTimeInterval)
	{
		// Calculate the FPS as the number of frames divided by the interval in seconds
		fps = (double)fpsFrameCount / (currentTime - t0Value);

		// Reset the FPS frame counter and set the initial time to be now
		fpsFrameCount = 0;
		t0Value = glfwGetTime();
	}
	else // FPS calculation time interval hasn't elapsed yet? Simply increment the FPS frame counter
	{
		fpsFrameCount++;
	}

	// Return the current FPS - doesn't have to be used if you don't want it!
	return fps;
}

void mainLoop() {
	const int samples = 60;
	float time[samples];
	int index = 0; 
	glfwSwapInterval(0);
	while (!glfwWindowShouldClose(window)) {

		// Calculate deltatime of current frame
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		do_movement();


		scene->update(float(glfwGetTime()));
		scene->draw(&camera);

		glfwSwapBuffers(window);

		// Update FPS
		std::stringstream strm;
		strm << title;
		strm.precision(4);
		strm << " (fps: " << calcFPS() << ")";
		glfwSetWindowTitle(window, strm.str().c_str());
	}
}

void resizeGL(int w, int h ) {
	scene->resize(w,h);
}

void window_size_callback(GLFWwindow* window, int w, int h) {
	camera.screenWidth = w;
	camera.screenHeight = h;
	resizeGL(w, h);
}

int main() {
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLU_TRUE);

	// Open the window
	title = "";
	window = glfwCreateWindow( WIN_WIDTH, WIN_HEIGHT, title.c_str(), NULL, NULL );
	
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	//glfwSetWindowSizeCallback(window, window_size_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Initialize GLEW to setup the OpenGL Function pointers
	glewExperimental = GL_TRUE;
	glewInit();

	// Initialization
	scene = new TransparencyScene(WIN_WIDTH, WIN_HEIGHT);
	scene->initScene();

	camera.screenWidth = WIN_WIDTH;
	camera.screenHeight = WIN_HEIGHT;

	resizeGL(WIN_WIDTH, WIN_HEIGHT);

	// Enter the main loop
	mainLoop();

	// Close window and terminate GLFW
	glfwTerminate();
	return 0;
}