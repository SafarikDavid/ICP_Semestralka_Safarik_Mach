#pragma once

#include <atomic>
#include <unordered_map>

#include <opencv2\opencv.hpp>
#include <GL/glew.h>
#include <GL/wglew.h>
#include <GLFW/glfw3.h>

#include "synced_deque.h"
#include "camera.h"
#include "ShaderProgram.h"
#include "Mesh.h"
#include "stb_image.h"


struct GameObject {
    glm::vec3 position = glm::vec3(0.);
    glm::vec3 dimensions = glm::vec3(1.0);
    Mesh mesh;
};

// our application class 
class App {
public:
    App(); // default constructor, called on app instance definition

    // public methods
    bool init(void);

    int run(void);

    void draw_cross_normalized(cv::Mat& img, cv::Point2f center_relative, int size);
    void draw_cross(cv::Mat& img, int x, int y, int size);
    cv::Point2f find_center_normalized(cv::Mat& frame);

    cv::Point2f find_center_normalized_hsv(cv::Mat& frame);

    ~App(); //default destructor, called on app instance destruction
private:
    void thread_code(void);

    void init_opencv();
    void init_glew(void);
    void init_glfw(void);
    void init_gl_debug();
    void init_assets(void);

    GLuint loadTexture(char const* path);
    GLuint gen_tex(const std::filesystem::path& file_name);

    void print_opencv_info();
    void print_glfw_info(void);
    void print_glm_info();
    void print_gl_info();

    //callbacks
    static void glfw_error_callback(int error, const char* description);
    static void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    static void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);


    void update_projection_matrix(void);

    void toggleFullscreen(GLFWwindow* window);

    uchar getmap(cv::Mat& map, int x, int y);
    void genLabyrinth(cv::Mat& map);

    bool checkCollision(const GameObject& obj1, const GameObject& obj2);

    cv::Mat mapa = cv::Mat(11, 11, CV_8U); // unsigned char

    cv::VideoCapture capture;
    synced_deque<cv::Point2f> fronta;
    std::atomic<bool> koncime = false;

    // GL
    GLFWwindow* window = { nullptr };
    GLuint shader_prog_ID = { 0 };
    GLuint VBO_ID = { 0 };
    GLuint VAO_ID = { 0 };
    int width{ 0 }, height{ 0 };
    glm::vec4 clear_color = glm::vec4(0.0f);
    glm::mat4 projection_matrix = glm::mat4(1.0f);
    int swap_interval = 1;
    float fov_degrees = 45.0f;

    bool firstMouse = true;
    float lastX, lastY, xoffset, yoffset;
    Camera camera = Camera(glm::vec3(0.0f, 5.0f, 10.0f));

    bool isFullscreen = false;
    
    std::unordered_map<std::string, GameObject> scene;

    GameObject playerObject;
};


