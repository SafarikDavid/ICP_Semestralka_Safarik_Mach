// icp.cpp 
// author: JJ

// C++ 
#include <iostream>
#include <atomic>
#include <chrono>
#include <vector>
#include <stack>
#include <random>
#include <numeric>

// OpenCV 
#include <opencv2\opencv.hpp>

// OpenGL Extension Wrangler
#include <GL/glew.h> 
#include <GL/wglew.h> //WGLEW = Windows GL Extension Wrangler (change for different platform) 

// GLFW toolkit
#include <GLFW/glfw3.h>

// OpenGL math
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// our awesome headers
#include "App.h"

// define our application
App app;

// MAIN program function
int main()
{
	if (app.init())
		return app.run();
}

/* */