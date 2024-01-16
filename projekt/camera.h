#include <glm/glm.hpp>

class Camera
{
public:
    enum class direction {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN
    };

    // Camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Right; 
    glm::vec3 Up; // camera local UP vector

    GLfloat Yaw = -90.0f;
    GLfloat Pitch =  0.0f;;
    GLfloat Roll = 0.0f;
    
    // Camera options
    GLfloat MovementSpeed = 1.0f;
    GLfloat MouseSensitivity = 0.25f;

    GLfloat camera_height = 0.5f;

    Camera() = default;

    Camera(glm::vec3 position):Position(position)
    {
        this->Up = glm::vec3(0.0f,1.0f,0.0f);
        // initialization of the camera reference system
        this->updateCameraVectors();
    }

    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(this->Position, this->Position + this->Front, this->Up);
    }

    glm::vec3 ProcessInputPoll(GLfloat deltaTime) {
        auto window = glfwGetCurrentContext();

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            return ProcessInput(Camera::direction::FORWARD, deltaTime);
        }

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            return ProcessInput(Camera::direction::BACKWARD, deltaTime);
        }

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            return ProcessInput(Camera::direction::LEFT, deltaTime);
        }

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            return ProcessInput(Camera::direction::RIGHT, deltaTime);
        }

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            return ProcessInput(Camera::direction::UP, deltaTime);
        }

        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
            return ProcessInput(Camera::direction::DOWN, deltaTime);
        }

        return glm::vec3(0.0f);
    }

    glm::vec3 ProcessInput(Camera::direction direction, GLfloat deltaTime)
    {
        GLfloat velocity = this->MovementSpeed * glm::abs(deltaTime);
        
        switch (direction)
        {
        case direction::FORWARD:
            return this->Front * velocity;
        case direction::BACKWARD:
            return -this->Front * velocity;
        case direction::LEFT:
            return -this->Right * velocity;
        case direction::RIGHT:
            return this->Right * velocity;
        case direction::UP:
            return this->Up * velocity;
        case direction::DOWN:
            return -this->Up * velocity;
        default:
            break;
        }
        return glm::vec3(0.0f);
    }

    void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constraintPitch = GL_TRUE)
    {
        xoffset *= this->MouseSensitivity;
        yoffset *= this->MouseSensitivity;

        this->Yaw   += xoffset;
        this->Pitch += yoffset;

        if (constraintPitch)
        {
            if (this->Pitch > 89.0f)
                this->Pitch = 89.0f;
            if (this->Pitch < -89.0f)
                this->Pitch = -89.0f;
        }

        this->updateCameraVectors();
    }

private:
    void updateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
        front.y = sin(glm::radians(this->Pitch));
        front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));

        this->Front = glm::normalize(front);
        this->Right = glm::normalize(glm::cross(this->Front, glm::vec3(0.0f,1.0f,0.0f)));
        this->Up    = glm::normalize(glm::cross(this->Right, this->Front));
    }
};
