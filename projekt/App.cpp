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
	// default constructor
	// nothing to do here (so far...)
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
	//{ // get extension list
	//    GLint n = 0;
	//    glGetIntegerv(GL_NUM_EXTENSIONS, &n);
	//    for (GLint i = 0; i < n; i++) {
	//        const char* extension_name = (const char*)glGetStringi(GL_EXTENSIONS, i);
	//        std::cout << extension_name << '\n';
	//    }
	//}
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
	window = glfwCreateWindow(800, 600, "Gamesa", NULL, NULL);
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

		std::cout << mapa;

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
	GLuint texture_box = loadTexture("resources/textures/box_rgb888.png");
	GLuint texture_floor = loadTexture("resources/textures/pavement.jpg");

	//ShaderProgram s("resources/shaders/obj.vert", "resources/shaders/obj.frag");

	// fully static objects - initialize all (including position) in init_assets()
	scene["plane"] = Mesh("resources/shaders/obj.vert", "resources/shaders/obj.frag", "resources/models/plane_tri_vnt.obj");
	scene["plane"].model_matrix = glm::translate(glm::identity<glm::mat4>(), glm::vec3(5, 0, 5));
	scene["plane"].diffuse_material = glm::vec4(glm::vec3(0.8), 1.0);
	scene["plane"].texture = texture_floor;

	auto temp_cube = Mesh("resources/shaders/obj.vert", "resources/shaders/obj.frag", "resources/models/cube_triangles_normals_tex.obj");
	temp_cube.texture = texture_box;

	// dynamic objects are initialized only partially
	scene["cube"] = temp_cube;
	scene["cube"].diffuse_material = glm::vec4(1.0f);

	//Labyrinth build
	for (auto cols = 0; cols < mapa.cols; ++cols) {
		for (auto rows = 0; rows < mapa.rows; ++rows) {
			switch (getmap(mapa, cols, rows)) {
				case '.':
					break;
				case 'e':
					temp_cube.diffuse_material = glm::vec4(glm::vec3(0.8, 0.4,0.4), 1.0);
					temp_cube.model_matrix = glm::translate(glm::identity<glm::mat4>(), glm::vec3(cols, 0.5f, rows));
					scene[std::string("bedna ").append(std::to_string(cols).append(";").append(std::to_string(rows)))] =
						temp_cube;
					break;
				case 'X':
					// player starting position
					break;
				case '#':
					temp_cube.diffuse_material = glm::vec4(glm::vec3(0.8), 1.0);
					temp_cube.model_matrix = glm::translate(glm::identity<glm::mat4>(), glm::vec3(cols, 0.5f, rows));
					scene[std::string("bedna ").append(std::to_string(cols).append(";").append(std::to_string(rows)))] = 
						temp_cube;
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

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
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

GLuint App::gen_tex(const std::filesystem::path& file_name)
{
	GLuint ID;
	cv::Mat image = cv::imread(file_name.string(), cv::IMREAD_UNCHANGED); // Read with (potential) Alpha

	// Generates an OpenGL texture object
	glGenTextures(1, &ID);

	// Assigns the texture to a Texture Unit
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ID);

	// Texture data alignment for transfer (single byte = basic, slow, but safe option; usually not necessary) 
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Assigns the image to the OpenGL Texture object
	switch (image.channels()) {
	case 3:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.cols, image.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, image.data);
		break;
	case 4:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.cols, image.rows, 0, GL_BGRA, GL_UNSIGNED_BYTE, image.data);
		break;
	default:
		throw std::exception("texture failed"); // Check the image, we want Alpha in this example    
	}



	// Configures the type of algorithm that is used to make the image smaller or bigger
	// nearest neighbor - ugly & fast 
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// bilinear - nicer & slower
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// MIPMAP filtering + automatic MIPMAP generation - nicest, needs more memory. Notice: MIPMAP is only for image minifying.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // bilinear magnifying
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // trilinear minifying
	glGenerateMipmap(GL_TEXTURE_2D);  //Generate mipmaps now.

	// Configures the way the texture repeats
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Unbinds the OpenGL Texture object so that it can't accidentally be modified
	glBindTexture(GL_TEXTURE_2D, 0);
	return ID;
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
		std::cerr << "no camera source? Fallback to video..." << std::endl;

		//open video file
		capture = cv::VideoCapture("resources/video.mkv");
		if (!capture.isOpened())
		{
			std::cerr << "no source?... " << std::endl;
			exit(EXIT_FAILURE);
		}
	}
}

void App::update_projection_matrix(void)
{
	if (height < 1)
		height = 1;

	float ratio = static_cast<float>(width) / height;

	projection_matrix = glm::perspective(
		glm::radians(fov_degrees), // The vertical Field of View, in radians: the amount of "zoom". Think "camera lens". Usually between 90� (extra wide) and 30� (quite zoomed in)
		ratio,			     // Aspect Ratio. Depends on the size of your window.
		0.1f,                // Near clipping plane. Keep as big as possible, or you'll get precision issues.
		20000.0f              // Far clipping plane. Keep as little as possible.
	);
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
	camera.Position.x = (start_position.x) + 1.0 / 2.0f;
	camera.Position.z = (start_position.y) + 1.0 / 2.0f;
	camera.Position.y = camera.camera_height;
}

int App::run(void)
{
	try {
		int framecnt = 0;
		double last_frame_time = glfwGetTime();
		double last_framecnt_time = last_frame_time;

		std::thread vlakno(&App::thread_code, this);

		glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
		
		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);

		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_POINT_SMOOTH);

		update_projection_matrix();
		glViewport(0, 0, width, height);

		camera.MovementSpeed = 3;
		//camera.Position = glm::vec3(0.0f, 5.0f, 10.0f);
		lastX = width/2;
		lastY = height/2;

		cv::Point2f tracker_normalized_center{ 0 };
		while (!glfwWindowShouldClose(window))
		{
			double now = glfwGetTime();
			double delta_t = now - last_frame_time;

			// thread related stuff
			{
				if (!fronta.empty()) {
					tracker_normalized_center = fronta.pop_front();
					//std::cout << "Normalized center found at: " << tracker_normalized_center << "\n";
					std::cout << '.';
				}
			}

			// process movement from keyboard, use poll method
			glm::vec3 offset = camera.ProcessInputPoll(delta_t);

			camera.Position.x += offset.x;
			camera.Position.z += offset.z;
			if (camera.Position.y + offset.y < 0.0 + camera.camera_height) {
				camera.Position.y = 0.0f + camera.camera_height;
			}
			else {
				camera.Position.y += offset.y;
			}

			// process mouse movements
			camera.ProcessMouseMovement(xoffset, yoffset);
			xoffset = 0; yoffset = 0;

			// OpenGL stuff...
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//view
			glm::mat4 v_m = camera.GetViewMatrix();

			//set Model matrix
			glm::mat4 m_m = glm::identity<glm::mat4>();

			m_m = glm::translate(m_m, 10.0f * glm::vec3(-0.5f, 0.0f, -0.5f)); //move to center of the plane
			m_m = glm::translate(m_m, glm::vec3(0, 0.5f, 0)); // move cube UP by half of its size (1.0), so it is not buried
			m_m = glm::translate(m_m, 10.0f * glm::vec3(tracker_normalized_center.x, 0.0f, tracker_normalized_center.y)); //move according to tracker
			m_m = glm::rotate(m_m, static_cast<float>(glfwGetTime()), glm::vec3(0.0f, 0.1f, 0.0f)); //rotate around axis Y
			scene["cube"].model_matrix = m_m;

			//draw whole scene
			for (auto& m : scene)
				m.second.draw(projection_matrix, v_m);

			glfwSwapBuffers(window);
			glfwPollEvents();
			last_frame_time = now;
			
			framecnt++;
			if ((now - last_framecnt_time) >= 1.0) {
				std::cout << "[FPS] " << framecnt << '\n';
				last_framecnt_time = now;
				framecnt = 0;
			}
		}

		koncime = true;
		vlakno.join();
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

	std::cout << "Bye...\n";
}

void App::thread_code(void)
{
	cv::Mat frame;

	try {
		while (true)
		{
			capture >> frame;

			if (frame.empty())
				throw std::exception("Empty file? Wrong path?");

			//cv::Point2f center_normalized = find_center_normalized(frame);
			cv::Point2f center_normalized = find_center_normalized_hsv(frame);

			fronta.push_back(center_normalized);

			if (koncime)
			{
				capture.release();
				break;
			}
		}
	}
	catch (std::exception const& e) {
		std::cerr << "App failed : " << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}
}

void App::draw_cross(cv::Mat& img, int x, int y, int size)
{
	cv::Point p1(x - size / 2, y);
	cv::Point p2(x + size / 2, y);
	cv::Point p3(x, y - size / 2);
	cv::Point p4(x, y + size / 2);

	cv::line(img, p1, p2, CV_RGB(255, 0, 0), 3);
	cv::line(img, p3, p4, CV_RGB(255, 0, 0), 3);
}

cv::Point2f App::find_center_normalized(cv::Mat& frame)
{
	// convert to grayscale, create threshold, sum white pixels
	// compute centroid of white pixels (average X,Y coordinate of all white pixels)
	cv::Point2f center;
	cv::Point2f center_normalized;

	//std::cout << "Center absolute: " << center << '\n';
	//std::cout << "Center normalized: " << center_normalized << '\n';

	return center_normalized;
}

cv::Point2f App::find_center_normalized_hsv(cv::Mat& frame)
{
	// convert to grayscale, create threshold, sum white pixels
	// compute centroid of white pixels (average X,Y coordinate of all white pixels)
	cv::Point2f center;
	cv::Point2f center_normalized;
	double h_low = 80.0;
	double s_low = 50.0;
	double v_low = 50.0;

	double h_hi = 100.0;
	double s_hi = 255.0;
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

void App::draw_cross_normalized(cv::Mat& img, cv::Point2f center_normalized, int size)
{
	center_normalized.x = std::clamp(center_normalized.x, 0.0f, 1.0f);
	center_normalized.y = std::clamp(center_normalized.y, 0.0f, 1.0f);
	//size = std::clamp(size, 1, std::min(img.cols, img.rows));

	cv::Point2f center_absolute(center_normalized.x * img.cols, center_normalized.y * img.rows);

	cv::Point2f p1(center_absolute.x - size / 2, center_absolute.y);
	cv::Point2f p2(center_absolute.x + size / 2, center_absolute.y);
	cv::Point2f p3(center_absolute.x, center_absolute.y - size / 2);
	cv::Point2f p4(center_absolute.x, center_absolute.y + size / 2);

	cv::line(img, p1, p2, CV_RGB(255, 0, 0), 3);
	cv::line(img, p3, p4, CV_RGB(255, 0, 0), 3);
}