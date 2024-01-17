#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "App.h"

void App::glfw_error_callback(int error, const char* description)
{
	std::cerr << "GLFW error: " << description << std::endl;
}

void App::glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	auto inst = static_cast<App*>(glfwGetWindowUserPointer(window));

	inst->width = width;
	inst->height = height;

	// set viewport
	glViewport(0, 0, width, height);
	//now your canvas has [0,0] in bottom left corner, and its size is [width x height] 

	inst->update_projection_matrix();
}

void App::glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	auto inst = static_cast<App*>(glfwGetWindowUserPointer(window));

	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			std::cout << "ESC has been pressed!\n";
			glfwSetWindowShouldClose(window, GLFW_TRUE);
			break;
		case GLFW_KEY_SPACE:
			inst->clear_color = glm::vec4(0.0f);
			glClearColor(inst->clear_color.r, inst->clear_color.g,
				inst->clear_color.b, inst->clear_color.a);
			break;
		case GLFW_KEY_V:
			if (inst->swap_interval != 0)
				inst->swap_interval = 0; //vsync off
			else
				inst->swap_interval = 1; //vsync on
			glfwSwapInterval(inst->swap_interval); // set vsync
			break;
		case GLFW_KEY_E:
			break;
		case GLFW_KEY_M:	// windowed/fullscreen
			inst->toggleFullscreen(window);
			break;
		default:
			break;
		}
	}
}

void App::glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	auto inst = static_cast<App*>(glfwGetWindowUserPointer(window));

	inst->fov_degrees += 10.0f * yoffset;
	inst->fov_degrees = std::clamp(inst->fov_degrees, 20.0f, 170.0f);

	inst->update_projection_matrix();
}

void App::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	auto inst = static_cast<App*>(glfwGetWindowUserPointer(window));

	if (inst->firstMouse)
	{
		inst->lastX = xpos;
		inst->lastY = ypos;
		inst->firstMouse = false;
	}

	inst->xoffset = xpos - inst->lastX;
	inst->yoffset = inst->lastY - ypos; // reversed since y-coordinates range from bottom to top
	inst->lastX = xpos;
	inst->lastY = ypos;
}

void GLAPIENTRY App::MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	auto const src_str = [source]() {
		switch (source)
		{
		case GL_DEBUG_SOURCE_API: return "API";
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW SYSTEM";
		case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
		case GL_DEBUG_SOURCE_THIRD_PARTY: return "THIRD PARTY";
		case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
		case GL_DEBUG_SOURCE_OTHER: return "OTHER";
		default: return "Unknown";
		}
		}();

		auto const type_str = [type]() {
			switch (type)
			{
			case GL_DEBUG_TYPE_ERROR: return "ERROR";
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
			case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
			case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
			case GL_DEBUG_TYPE_MARKER: return "MARKER";
			case GL_DEBUG_TYPE_OTHER: return "OTHER";
			default: return "Unknown";
			}
			}();

			auto const severity_str = [severity]() {
				switch (severity) {
				case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
				case GL_DEBUG_SEVERITY_LOW: return "LOW";
				case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
				case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
				default: return "Unknown";
				}
				}();

				std::cout << "[GL CALLBACK]: " <<
					"source = " << src_str <<
					", type = " << type_str <<
					", severity = " << severity_str <<
					", ID = '" << id << '\'' <<
					", message = '" << message << '\'' << std::endl;
}
