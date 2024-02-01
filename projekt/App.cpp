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
	playerObject.dimensions = glm::vec3(0.5);

	GLuint texture_box = loadTexture("resources/textures/box_rgb888.png");
	GLuint texture_floor = loadTexture("resources/textures/pavement.jpg");
	//GLuint texture_final_box = loadTexture("resources/textures/brick_wall-red.png"); 
	GLuint texture_final_box = loadTexture("resources/textures/window.png");
	GLuint texture_bunny = loadTexture("resources/textures/brick_wall-red.png");
	GLuint texture_ball = loadTexture("resources/textures/green_metal_rust.jpg");


	//ShaderProgram s("resources/shaders/obj.vert", "resources/shaders/obj.frag");

	//scene["bunny"] = Mesh("resources/shaders/obj.vert", "resources/shaders/obj.frag", "resources/models/female_secretary/female_secretary.fbx");
	scene["bunny"].mesh = Mesh("resources/shaders/obj.vert", "resources/shaders/obj.frag", "resources/models/bunny_tri_vnt.obj");
	scene["bunny"].position = glm::vec3(0, 2, 0);
	scene["bunny"].dimensions = scene["bunny"].mesh.calculateDimensions(0.2f);
	scene["bunny"].mesh.model_matrix = glm::scale(glm::translate(glm::identity<glm::mat4>(), scene["bunny"].position), glm::vec3(0.2f));
	scene["bunny"].mesh.specular_material = glm::vec4(glm::vec3(0.8), 1.0);
	scene["bunny"].mesh.shininess = 32.0f;
	scene["bunny"].mesh.texture = texture_bunny;

	scene["ball"].mesh = Mesh("resources/shaders/obj.vert", "resources/shaders/obj.frag", "resources/models/teapot_tri_vnt.obj");
	scene["ball"].position = glm::vec3(0, 2, 5);
	scene["ball"].dimensions = scene["ball"].mesh.calculateDimensions(0.2);
	scene["ball"].mesh.model_matrix = glm::scale(glm::translate(glm::identity<glm::mat4>(), scene["ball"].position), glm::vec3(0.2f));
	scene["ball"].mesh.specular_material = glm::vec4(glm::vec3(0.8), 1.0);
	scene["ball"].mesh.shininess = 32.0f;
	scene["ball"].mesh.texture = texture_ball;

	// fully static objects - initialize all (including position) in init_assets()
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


	// dynamic objects are initialized only partially
	scene["cube"].mesh = temp_cube;
	scene["cube"].mesh.specular_material = glm::vec4(1.0);
	scene["cube"].mesh.shininess = 3.0f;

	std::string key_val;
	//Labyrinth build
	for (auto cols = 0; cols < mapa.cols; ++cols) {
		for (auto rows = 0; rows < mapa.rows; ++rows) {
			switch (getmap(mapa, cols, rows)) {
				case '.':
					break;
				case 'e':
					// end_cube.diffuse_material = glm::vec4(glm::vec3(0.8, 0.4, 0.4), 1.0);
					// end_cube.ambient_material = glm::vec4(glm::vec3(0.8, 0.4, 0.4), 1.0);
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
					// temp_cube.diffuse_material = glm::vec4(glm::vec3(0.8), 1.0);
					// temp_cube.ambient_material = glm::vec4(glm::vec3(0.8), 1.0);
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
		glm::radians(fov_degrees), // The vertical Field of View, in radians: the amount of "zoom". Think "camera lens". Usually between 90° (extra wide) and 30° (quite zoomed in)
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

bool App::checkCollision(const GameObject& obj1, const GameObject& obj2){
	return (obj1.position.x < obj2.position.x + obj2.dimensions.x &&
		obj1.position.x + obj1.dimensions.x > obj2.position.x &&
		obj1.position.y < obj2.position.y + obj2.dimensions.y &&
		obj1.position.y + obj1.dimensions.y > obj2.position.y &&
		obj1.position.z < obj2.position.z + obj2.dimensions.z &&
		obj1.position.z + obj1.dimensions.z > obj2.position.z);
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

		// Enable blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	// makes alpha component be influenced by the source's alpha value

		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_POINT_SMOOTH);

		update_projection_matrix();
		glViewport(0, 0, width, height);

		camera.MovementSpeed = 3;
		//camera.Position = glm::vec3(0.0f, 5.0f, 10.0f);
		lastX = width/2;
		lastY = height/2;

		// update position of player object
		playerObject.position = camera.Position;

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
				offset.y = 0;
			}
			else {
				camera.Position.y += offset.y;
			}

			// update position of player object
			playerObject.position = camera.Position;

			// collision detection loop
			for (auto& scene_object : scene) {
				if (checkCollision(playerObject, scene_object.second)) {
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
			scene["cube"].mesh.model_matrix = m_m;


			// Draw all objects except end point ("bedna konec")
			for (auto& scene_object : scene) {
				if (scene_object.first != "bedna konec") {
					scene_object.second.mesh.viewPos = camera.Position;
					scene_object.second.mesh.viewFront = camera.Front;
					scene_object.second.mesh.draw(projection_matrix, v_m);
				}
			}

			// Draw end point last
			auto end_point_iter = scene.find("bedna konec");
			if (end_point_iter != scene.end()) {
				end_point_iter->second.mesh.viewPos = camera.Position;
				end_point_iter->second.mesh.viewFront = camera.Front;
				end_point_iter->second.mesh.draw(projection_matrix, v_m);
			}

			glfwSwapBuffers(window);
			glfwPollEvents();
			last_frame_time = now;
			
			framecnt++;
			if ((now - last_framecnt_time) >= 1.0) {
				std::cout << "[FPS] " << framecnt << '\n';
				std::cout << "Pos: " << camera.Position[0] << " " << camera.Position[1] << " " << camera.Position[2] << '\n';
				std::cout << "Front: " << camera.Front[0] << " " << camera.Front[1] << " " << camera.Front[2] << '\n';
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

	std::cout << "Game Ended...\n";
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
