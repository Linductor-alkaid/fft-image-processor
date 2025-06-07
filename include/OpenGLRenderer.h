#ifndef OPENGL_RENDERER_H
#define OPENGL_RENDERER_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

class OpenGLRenderer {
private:
    GLFWwindow* window;
    GLuint shaderProgram, imageShaderProgram, axisShaderProgram;
    GLuint VAO, VBO, imageVAO, imageVBO, axisVAO, axisVBO;
    GLuint textureID;
    glm::mat4 view, projection;
    float cameraAngle;
    int displayMode; // 0: 原图, 1: 频域3D, 2: 重建图
    
    // 鼠标控制相关
    bool mousePressed;
    double lastMouseX, lastMouseY;
    float cameraRadius;
    float cameraTheta, cameraPhi; // 球坐标
    glm::vec3 cameraCenter; // 摄像头围绕的中心点
    
    // 着色器源码
    const char* vertexShaderSource;
    const char* fragmentShaderSource;
    const char* imageVertexShaderSource;
    const char* imageFragmentShaderSource;
    const char* axisVertexShaderSource;
    const char* axisFragmentShaderSource;
    
    // 私有方法
    GLuint compileShader(GLenum type, const char* source);
    void setupImageQuad();
    void setupAxisData();
    void renderImage(const std::vector<std::vector<double>>& imageData, int width, int height);
    void updateCamera();
    void calculateCameraCenter(const std::vector<float>& vertices);
    
public:
    OpenGLRenderer();
    ~OpenGLRenderer();
    
    // 禁用拷贝构造和赋值操作（因为包含OpenGL资源）
    OpenGLRenderer(const OpenGLRenderer&) = delete;
    OpenGLRenderer& operator=(const OpenGLRenderer&) = delete;
    
    bool initialize();
    bool initializeWithoutWindow();
    void render3DFrequency(int pointCount);
    void renderAxes();
    void handleMouseDrag(float deltaX, float deltaY);
    void handleMouseWheel(float delta);
    void setupVertexData(const std::vector<float>& vertices);
    void setupImageTexture(const std::vector<std::vector<double>>& imageData, int width, int height);
    void updateImageTexture(const std::vector<std::vector<double>>& imageData, int width, int height);
    
    void render(int pointCount, 
               const std::vector<std::vector<double>>& originalImage, 
               const std::vector<std::vector<double>>& reconstructedImage, 
               int width, int height);
    
    void cleanup();
    bool shouldClose();
    
    // 键盘回调函数
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    
    // 鼠标回调函数
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
};

#endif // OPENGL_RENDERER_H