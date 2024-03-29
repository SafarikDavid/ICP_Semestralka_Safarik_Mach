#include <thread>
#include <chrono>
#include <string>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <random>

#include <glm/glm.hpp>
#include <glm/ext.hpp>


#include "App.h"

App::App()
{
}

void App::init_glew(void) {
	//
	// Initialize all valid generic GL extensions with GLEW.
	// Usable AFTER creating GL context! (create with glfwInit(), glfwCreateWindow(), glfwMakeContextCurrent()
	//
	{
		GLenum glew_ret;
		glew_ret = glewInit();
		if (glew_ret != GLEW_OK) {
			throw std::exception(std::string("GLEW failed with error: ").append((const char*)glewGetErrorString(glew_ret)).append("\n").c_str());
		}
		else {
			std::cout << "GLEW successfully initialized to version: " << glewGetString(GLEW_VERSION) << "\n";
		}

		// Platform specific init. (Change to GLXEW or ELGEW if necessary.)
		glew_ret = wglewInit();
		if (glew_ret != GLEW_OK) {
			throw std::exception(std::string("WGLEW failed with error: ").append((const char*)glewGetErrorString(glew_ret)).append("\n").c_str());
		}
		else {
			std::cout << "WGLEW successfully initialized platform specific functions.\n";
		}
	}
}

void App::init_glfw(void)
{

	/* Initialize the library */
	glfwSetErrorCallback(glfw_error_callback);

	if (!glfwInit()) {
		throw std::exception("GLFW can not be initialized.");
	}

	// try to open OpenGL 4.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(800, 600, "Game", NULL, NULL);
	if (!window) {
		throw std::exception("GLFW window can not be created.");
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

bool App::init()
{
	try {
		std::cout << "Current working directory: " << std::filesystem::current_path().generic_string() << '\n';

		if (!std::filesystem::exists("bin"))
			throw std::exception("Directory 'bin' not found. DLLs are expected to be there.");

		if (!std::filesystem::exists("resources"))
			throw std::exception("Directory 'resources' not found. Various media files are expected to be there.");
		
		// some init
		// if (not_success)
		//  throw std::exception("something went bad");

		init_opencv();
		init_glfw();
		init_glew();

		init_gl_debug();

		print_opencv_info();
		print_glfw_info();
		print_gl_info();
		print_glm_info();

		glfwSwapInterval(swap_interval); // vsync

		// GLFW callbacks registration
		glfwSetKeyCallback(window, glfw_key_callback);
		glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);
		glfwSetScrollCallback(window, glfw_scroll_callback);
		glfwSetCursorPosCallback(window, mouse_callback);

		glfwGetFramebufferSize(window, &width, &height);
		glfwSetWindowUserPointer(window, this);

		genLabyrinth(mapa);

	init_assets();
	}
	catch (std::exception const& e) {
		std::cerr << "Init failed : " << e.what() << std::endl;
		throw;
	}

	return true;
}

void App::init_gl_debug()
{
	if (GLEW_ARB_debug_output)
	{
		glDebugMessageCallback(MessageCallback, 0);
		glEnable(GL_DEBUG_OUTPUT);
		std::cout << "GL_DEBUG enabled." << std::endl;
	}
}

void App::init_assets(void)
{
	// set player bounding box dimensions
	playerObject.dimensions = glm::vec3(0.5);

	// texture loading
	GLuint texture_box = loadTexture("resources/textures/box_rgb888.png");
	GLuint texture_floor = loadTexture("resources/textures/pavement.jpg");
	GLuint texture_final_box = loadTexture("resources/textures/window.png");
	GLuint texture_bunny = loadTexture("resources/textures/brick_wall-red.png");
	GLuint texture_teapot = loadTexture("resources/textures/green_metal_rust.jpg");
	GLuint texture_flipcow = loadTexture("resources/textures/factory_wall_diff_4k.jpg");

	// Scene creation
	scene["bunny"].mesh = Mesh("resources/shaders/obj.vert", "resources/shaders/obj.frag", "resources/models/bunny_tri_vnt.obj");
	scene["bunny"].position = glm::vec3(0, 2, 0);
	scene["bunny"].dimensions = scene["bunny"].mesh.calculateDimensions(0.2f);
	scene["bunny"].mesh.model_matrix = glm::scale(glm::translate(glm::identity<glm::mat4>(), scene["bunny"].position), glm::vec3(0.2f));
	scene["bunny"].mesh.specular_material = glm::vec4(glm::vec3(0.8), 1.0);
	scene["bunny"].mesh.shininess = 32.0f;
	scene["bunny"].mesh.texture = texture_bunny;

	scene["teapot"].mesh = Mesh("resources/shaders/obj.vert", "resources/shaders/obj.frag", "resources/models/teapot_tri_vnt.obj");
	scene["teapot"].position = glm::vec3(0, 2, 5);
	scene["teapot"].dimensions = scene["teapot"].mesh.calculateDimensions(0.2);
	scene["teapot"].mesh.model_matrix = glm::scale(glm::translate(glm::identity<glm::mat4>(), scene["teapot"].position), glm::vec3(0.2f));
	scene["teapot"].mesh.specular_material = glm::vec4(glm::vec3(0.8), 1.0);
	scene["teapot"].mesh.shininess = 32.0f;
	scene["teapot"].mesh.texture = texture_teapot;

	scene["suzanne"].mesh = Mesh("resources/shaders/obj.vert", "resources/shaders/obj.frag", "resources/models/suzanne.obj");
	scene["suzanne"].position = glm::vec3(7, 4, 9);
	scene["suzanne"].dimensions = scene["suzanne"].mesh.calculateDimensions(0.2);
	scene["suzanne"].mesh.model_matrix = glm::scale(glm::translate(glm::identity<glm::mat4>(), scene["suzanne"].position), glm::vec3(0.5f));
	scene["suzanne"].mesh.specular_material = glm::vec4(glm::vec3(0.8), 1.0);
	scene["suzanne"].mesh.shininess = 32.0f;
	scene["suzanne"].mesh.texture = texture_flipcow;

	scene["plane"].mesh = Mesh("resources/shaders/obj.vert", "resources/shaders/obj.frag", "resources/models/plane_tri_vnt.obj");
	scene["plane"].position = glm::vec3(5, 0, 5);
	scene["plane"].dimensions = scene["plane"].mesh.calculateDimensions();
	scene["plane"].mesh.model_matrix = glm::scale(glm::translate(glm::identity<glm::mat4>(), scene["plane"].position), glm::vec3(1.f));
	scene["plane"].mesh.specular_material = glm::vec4(glm::vec3(0.8), 1.0);
	scene["plane"].mesh.shininess = 1.0f;
	scene["plane"].mesh.texture = texture_floor;

	auto temp_cube = Mesh("resources/shaders/obj.vert", "resources/shaders/obj.frag", "resources/models/cube_triangles_normals_tex.obj");
	temp_cube.texture = texture_box;

	auto end_cube = Mesh("resources/shaders/obj.vert", "resources/shaders/obj.frag", "resources/models/cube_triangles_normals_tex.obj");
	end_cube.texture = texture_final_box;
	std::string end_cube_identification;


	std::string key_val;
	//Labyrinth build
	for (auto cols = 0; cols < mapa.cols; ++cols) {
		for (auto rows = 0; rows < mapa.rows; ++rows) {
			switch (getmap(mapa, cols, rows)) {
				case '.':
					break;
				case 'e':
					end_cube.specular_material = glm::vec4(1.0);
					end_cube.shininess = 0.5f;
					end_cube.model_matrix = glm::translate(glm::identity<glm::mat4>(), glm::vec3(cols, 0.5f, rows));
					end_cube_identification = std::string("bedna konec");
					scene[end_cube_identification].mesh = end_cube;
					scene[end_cube_identification].position = glm::vec3(cols, 0.5f, rows);
					scene[end_cube_identification].dimensions = scene[end_cube_identification].mesh.calculateDimensions();
					break;
				case 'X':
					// player starting position
					break;
				case '#':
					key_val = std::string("bedna ").append(std::to_string(cols).append(";").append(std::to_string(rows)));
					temp_cube.specular_material = glm::vec4(glm::vec3(0.8), 1.0);
					temp_cube.shininess = 0.5f;
					temp_cube.model_matrix = glm::translate(glm::identity<glm::mat4>(), glm::vec3(cols, 0.5f, rows));
					scene[key_val].mesh = temp_cube;
					scene[key_val].position = glm::vec3(cols, 0.5f, rows);
					scene[key_val].dimensions = scene[key_val].mesh.calculateDimensions();
					break;
				default:
					break;
			}
		}
	}
}

GLuint App::loadTexture(char const* path)
{
	GLuint textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format{};
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

void App::print_opencv_info()
{
	std::cout << "Capture source: " <<
		": width=" << capture.get(cv::CAP_PROP_FRAME_WIDTH) <<
		", height=" << capture.get(cv::CAP_PROP_FRAME_HEIGHT) << '\n';
}

void App::print_glfw_info(void)
{
	int major, minor, revision;
	glfwGetVersion(&major, &minor, &revision);
	std::cout << "Running GLFW " << major << '.' << minor << '.' << revision << '\n';
	std::cout << "Compiled against GLFW "
		<< GLFW_VERSION_MAJOR << '.' << GLFW_VERSION_MINOR << '.' << GLFW_VERSION_REVISION
		<< '\n';
}

void App::print_glm_info()
{
	// GLM library
	std::cout << "GLM version: " << GLM_VERSION_MAJOR << '.' << GLM_VERSION_MINOR << '.' << GLM_VERSION_PATCH << "rev" << GLM_VERSION_REVISION << std::endl;
}

void App::print_gl_info()
{
	// get OpenGL info
	auto vendor_s = (const char*)glGetString(GL_VENDOR);
	std::cout << "OpenGL driver vendor: " << (vendor_s ? vendor_s : "UNKNOWN") << '\n';

	auto renderer_s = (const char*)glGetString(GL_RENDERER);
	std::cout << "OpenGL renderer: " << (renderer_s ? renderer_s : "<UNKNOWN>") << '\n';

	auto version_s = (const char*)glGetString(GL_VERSION);
	std::cout << "OpenGL version: " << (version_s ? version_s : "<UNKNOWN>") << '\n';

	auto glsl_s = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
	std::cout << "Primary GLSL shading language version: " << (glsl_s ? glsl_s : "<UNKNOWN>") << std::endl;

	// get GL profile info
	{
		GLint profile_flags;
		glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile_flags);
		std::cout << "Current profile: ";
		if (profile_flags & GL_CONTEXT_CORE_PROFILE_BIT)
			std::cout << "CORE";
		else
			std::cout << "COMPATIBILITY";
		std::cout << std::endl;
	}

	// get context flags
	{
		GLint context_flags;
		glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
		std::cout << "Active context flags: ";
		if (context_flags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)
			std::cout << "GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT ";
		if (context_flags & GL_CONTEXT_FLAG_DEBUG_BIT)
			std::cout << "GL_CONTEXT_FLAG_DEBUG_BIT ";
		if (context_flags & GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT)
			std::cout << "GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT ";
		if (context_flags & GL_CONTEXT_FLAG_NO_ERROR_BIT)
			std::cout << "GL_CONTEXT_FLAG_NO_ERROR_BIT";
		std::cout << std::endl;
	}
}

void App::init_opencv()
{
	//open first available camera
	capture = cv::VideoCapture(cv::CAP_DSHOW);

	if (!capture.isOpened())
	{
		std::cerr << "No camera source? Tracker feature disabled." << std::endl;

		videoAvailable = false;
	}
}

void App::update_projection_matrix(void)
{
	if (height < 1)
		height = 1;

	float ratio = static_cast<float>(width) / height;

	projection_matrix = glm::perspective(
		glm::radians(fov_degrees), // The vertical Field of View, in radians: the amount of "zoom". Usually between 90� (extra wide) and 30� (quite zoomed in)
		ratio,			     // Aspect Ratio. Depends on the size of your window.
		0.1f,                // Near clipping plane. Keep as big as possible, or you'll get precision issues.
		20000.0f              // Far clipping plane. Keep as little as possible.
	);
}

void App::process_object_movement(GLfloat deltaTime){
	// Moving Objects
	// Bunny - Moving
	float movementDirection = (endPosBool) ? 1.0f : -1.0f;
	scene["bunny"].position.x += movementDirection * superSpeed * glm::abs(deltaTime);
	scene["bunny"].position.y += movementDirection * superSpeed * glm::abs(deltaTime);
	scene["bunny"].mesh.model_matrix = glm::scale(glm::translate(glm::identity<glm::mat4>(), scene["bunny"].position), glm::vec3(0.2f));

	if ((endPosBool && scene["bunny"].position.x >= bunnyPositiveCap) ||
		(!endPosBool && scene["bunny"].position.x <= bunnyNegativeCap)) {
		endPosBool = !endPosBool;
	}
	// Teapot - Rotation
	//trans = glm::rotate(ma4_for_rotation, angle_in_radians, glm::vec3(0.0f, 0.0f, 1.0f) - osy rotace);
	scene["teapot"].mesh.model_matrix = glm::rotate(scene["teapot"].mesh.model_matrix, glm::radians(rotationAngle * glm::abs(deltaTime)), glm::vec3(0.0f, 1.0f, 1.0f));

	// Suzanne - moving
	scene["suzanne"].position.z += movementDirection * superSpeed * glm::abs(deltaTime);
	scene["suzanne"].mesh.model_matrix = glm::translate(glm::identity<glm::mat4>(), scene["suzanne"].position);
}

void App::toggleFullscreen(GLFWwindow* window)
{
	if (!isFullscreen) {
		// Switch to fullscreen mode
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	}
	else {
		// Switch to windowed mode
		glfwSetWindowMonitor(window, nullptr, 100, 100, 800, 600, GLFW_DONT_CARE);
	}
	isFullscreen = !isFullscreen;
	glfwSwapInterval(swap_interval); // set vsync

	firstMouse = true;
	lastX = width / 2;
	lastY = height / 2;	
}

// Secure access to map
uchar App::getmap(cv::Mat& map, int x, int y)
{
	x = std::clamp(x, 0, map.cols);
	y = std::clamp(y, 0, map.rows);

	//at(row,col)!!!
	return map.at<uchar>(y, x);
}

// Random map gen
void App::genLabyrinth(cv::Mat& map) {
	cv::Point2i start_position, end_position;

	// C++ random numbers
	std::random_device r; // Seed with a real random value, if available
	std::default_random_engine e1(r());
	std::uniform_int_distribution<int> uniform_height(1, map.rows - 2); // uniform distribution between int..int
	std::uniform_int_distribution<int> uniform_width(1, map.cols - 2);
	std::uniform_int_distribution<int> uniform_block(0, 7); // how often are walls generated: 0=wall, anything else=empty

	//inner maze 
	for (int j = 0; j < map.rows; j++) {
		for (int i = 0; i < map.cols; i++) {
			switch (uniform_block(e1))
			{
			case 0:
				map.at<uchar>(cv::Point(i, j)) = '#';
				break;
			default:
				map.at<uchar>(cv::Point(i, j)) = '.';
				break;
			}
		}
	}

	//walls
	for (int i = 0; i < map.cols; i++) {
		map.at<uchar>(cv::Point(i, 0)) = '#';
		map.at<uchar>(cv::Point(i, map.rows - 1)) = '#';
	}
	for (int j = 0; j < map.rows; j++) {
		map.at<uchar>(cv::Point(0, j)) = '#';
		map.at<uchar>(cv::Point(map.cols - 1, j)) = '#';
	}

	//gen start_position inside maze (excluding walls)
	do {
		start_position.x = uniform_width(e1);
		start_position.y = uniform_height(e1);
	} while (getmap(map, start_position.x, start_position.y) == '#'); //check wall

	//gen end different from start, inside maze (excluding outer walls) 
	do {
		end_position.x = uniform_width(e1);
		end_position.y = uniform_height(e1);
	} while (start_position == end_position); //check overlap
	map.at<uchar>(cv::Point(end_position.x, end_position.y)) = 'e';

	std::cout << "Start: " << start_position << std::endl;
	std::cout << "End: " << end_position << std::endl;

	//print map
	for (int j = 0; j < map.rows; j++) {
		for (int i = 0; i < map.cols; i++) {
			if ((i == start_position.x) && (j == start_position.y))
				std::cout << 'X';
			else
				std::cout << getmap(map, i, j);
		}
		std::cout << std::endl;
	}

	//set player position in 3D space (transform X-Y in map to XYZ in GL)
	camera.Position.x = (start_position.x);
	camera.Position.z = (start_position.y);
	camera.Position.y = camera.camera_height;
}

bool App::checkCollision(const GameObject& obj1, const GameObject& obj2){
	return (
		obj1.position.x - obj1.dimensions.x / 2 < obj2.position.x + obj2.dimensions.x / 2 &&
		obj1.position.x + obj1.dimensions.x / 2 > obj2.position.x - obj2.dimensions.x / 2 &&
		obj1.position.y - obj1.dimensions.y / 2 < obj2.position.y + obj2.dimensions.y / 2 &&
		obj1.position.y + obj1.dimensions.y / 2 > obj2.position.y - obj2.dimensions.y / 2 &&
		obj1.position.z - obj1.dimensions.z / 2 < obj2.position.z + obj2.dimensions.z / 2 &&
		obj1.position.z + obj1.dimensions.z / 2 > obj2.position.z - obj2.dimensions.z / 2
		);
}

int App::run(void)
{
	try {
		int framecnt = 0;
		double last_frame_time = glfwGetTime();
		double last_framecnt_time = last_frame_time;

		// start thread 
		std::thread tracker_thread(&App::tracker_thread_code, this);

		glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
		
		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);

		// Enable blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	// makes alpha component be influenced by the source's alpha value

		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_POINT_SMOOTH);

		update_projection_matrix();
		glViewport(0, 0, width, height);

		// update camera parameters
		camera.MovementSpeed = 3;
		lastX = width/2;
		lastY = height/2;

		// update position of player object to match camera's position
		playerObject.position = camera.Position;

		cv::Point2f tracker_normalized_center{ 0 };
		while (!glfwWindowShouldClose(window))
		{
			// get new time
			double now = glfwGetTime();
			double delta_t = now - last_frame_time;

			// thread related stuff
			if (videoAvailable && !fronta.empty()) {
				tracker_normalized_center = fronta.pop_front();
				std::cout << '.';
			}

			// process movement from keyboard, use poll method
			glm::vec3 offset = camera.ProcessInputPoll(delta_t);

			camera.Position.x += offset.x;
			camera.Position.z += offset.z;
			// cannot move into negative position on the y axis
			if (camera.Position.y + offset.y < 0.0 + camera.camera_height) {
				camera.Position.y = 0.0f + camera.camera_height;
				offset.y = 0;
			}
			else {
				camera.Position.y += offset.y;
			}

			// Moving Objects - update object positions
			process_object_movement(delta_t);

			// update position of player object
			playerObject.position = camera.Position;

			// collision detection loop
			for (auto& scene_object : scene) {
				if (checkCollision(playerObject, scene_object.second)) {
					// collision resolution - reverting movement
					// better would be to calculate distance to perfect collision
					camera.Position.x -= offset.x;
					camera.Position.z -= offset.z;
					camera.Position.y -= offset.y;
					break;
				}
			}

			// update position of player object again after checking collisions
			playerObject.position = camera.Position;

			// process mouse movements
			camera.ProcessMouseMovement(xoffset, yoffset);
			xoffset = 0; yoffset = 0; // set offsets to zero to eliminate residual values

			// OpenGL stuff...
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//view
			glm::mat4 view_matrix = camera.GetViewMatrix();

			//flashlight tracker
			glm::vec3 flashLightDirection;
			if (videoAvailable && trackFlashlight) {
				// calculate offset from the camera view based on the tracker position
				float tracker_x = tracker_normalized_center.x;
				float tracker_y = tracker_normalized_center.y;
				float tracker_x_offset = 0.5 - tracker_x;
				float tracker_y_offset = 0.5 - tracker_y;
				// the ammount of tracker flashlight offset is dependent on fov
				float tracker_yaw = camera.Yaw + (tracker_x_offset * fov_degrees);
				float tracker_pitch = camera.Pitch + (tracker_y_offset * fov_degrees);

				flashLightDirection.x = cos(glm::radians(tracker_yaw)) * cos(glm::radians(tracker_pitch));
				flashLightDirection.y = sin(glm::radians(tracker_pitch));
				flashLightDirection.z = sin(glm::radians(tracker_yaw)) * cos(glm::radians(tracker_pitch));

				flashLightDirection = glm::normalize(flashLightDirection);

				//check if result in NaN
				if (std::isnan(flashLightDirection.x) || std::isnan(flashLightDirection.y) || std::isnan(flashLightDirection.z)){
					flashLightDirection.x = camera.Front.x;
					flashLightDirection.y = camera.Front.y;
					flashLightDirection.z = camera.Front.z;
				}
			}else{
				flashLightDirection.x = camera.Front.x;
				flashLightDirection.y = camera.Front.y;
				flashLightDirection.z = camera.Front.z;

			}

			// Draw all objects except end point ("bedna konec")
			for (auto& scene_object : scene) {
				if (scene_object.first != "bedna konec") 
				{
					scene_object.second.mesh.viewPos = camera.Position;
					scene_object.second.mesh.flashLightDirection = flashLightDirection;
					scene_object.second.mesh.draw(projection_matrix, view_matrix);
				}
			}

			// Draw end point last - for correct transparency
			auto end_point_iter = scene.find("bedna konec");
			if (end_point_iter != scene.end()) 
			{
				end_point_iter->second.mesh.viewPos = camera.Position;
				end_point_iter->second.mesh.flashLightDirection = flashLightDirection;
				end_point_iter->second.mesh.draw(projection_matrix, view_matrix);
			}

			glfwSwapBuffers(window);
			glfwPollEvents();
			last_frame_time = now;
			
			framecnt++;
			if ((now - last_framecnt_time) >= 1.0) {
				std::cout << "[FPS] " << framecnt << std::endl;
				last_framecnt_time = now;
				framecnt = 0;
			}
		}

		thread_should_end = true;
		tracker_thread.join();
	}
	catch (std::exception const& e) {
		std::cerr << "App failed : " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

App::~App()
{
	// clean-up OpenCV
	if (capture.isOpened())
		capture.release();

	cv::destroyAllWindows();
	
	// clean-up GLFW
	glfwTerminate();

	std::cout << "Game Ended...\n";
}

void App::tracker_thread_code(void)
{
	// No video?
	if (!videoAvailable) return;

	cv::Mat frame;

	try {
		while (true)
		{
			capture >> frame;

			if (frame.empty())
				throw std::exception("Empty file? Wrong path?");

			cv::Point2f center_normalized = find_center_normalized_hsv(frame);

			fronta.push_back(center_normalized);

			if (thread_should_end)
			{
				capture.release();
				break;
			}
		}
	}
	catch (std::exception const& e) {
		std::cerr << "App failed in thread : " << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}
}

cv::Point2f App::find_center_normalized_hsv(cv::Mat& frame)
{
	// convert to grayscale, create threshold, sum white pixels
	// compute centroid of white pixels (average X,Y coordinate of all white pixels)
	cv::Point2f center;
	cv::Point2f center_normalized;

	//Nastaveni hledane barvy
	//hue
	double h_low = 25.0;
	double h_hi = 35.0;
	//saturation (y osa v HSV-MAP.png)
	double s_low = 75.0;
	double s_hi = 255.0;
	//value ("z" osa v HSV-MAP.png)
	double v_low = 50.0;
	double v_hi = 255.0;

	cv::Mat scene_hsv, scene_threshold;

	cv::cvtColor(frame, scene_hsv, cv::COLOR_BGR2HSV);

	cv::Scalar lower_threshold = cv::Scalar(h_low, s_low, v_low);
	cv::Scalar upper_threshold = cv::Scalar(h_hi, s_hi, v_hi);
	cv::inRange(scene_hsv, lower_threshold, upper_threshold, scene_threshold);

	int sy = 0, sx = 0, s = 0;
	for (int y = 0; y < frame.rows; y++) //y
	{
		for (int x = 0; x < frame.cols; x++) //x
		{
			// FIND THRESHOLD (value 0..255)
			if (scene_threshold.at<unsigned char>(y, x) < 255) {
				// set output pixel black

			}
			else {
				// set output pixel white
				sx += x;
				sy += y;
				s++;
			}
		}
	}

	center = cv::Point2f(sx / (float)s, sy / (float)s);
	center_normalized = cv::Point2f(center.x / frame.cols, center.y / frame.rows);

	//std::cout << "Center absolute: " << center << '\n';
	//std::cout << "Center normalized: " << center_normalized << '\n';

	return center_normalized;
}