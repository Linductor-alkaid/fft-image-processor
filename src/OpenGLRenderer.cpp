#include "OpenGLRenderer.h"
#include <iostream>
#include <algorithm>
#include <cmath>

OpenGLRenderer::OpenGLRenderer() 
    : window(nullptr), shaderProgram(0), imageShaderProgram(0), axisShaderProgram(0),
      VAO(0), VBO(0), imageVAO(0), imageVBO(0), axisVAO(0), axisVBO(0), textureID(0),
      displayMode(0), // 移除cameraAngle变量
      mousePressed(false), lastMouseX(0.0), lastMouseY(0.0),
      cameraRadius(5.0f), cameraTheta(45.0f), cameraPhi(30.0f),
      cameraCenter(0.0f, 0.0f, 0.0f) {
    
    // 3D 点云着色器源码
    vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aColor;
    
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    
    out vec3 vertexColor;
    
    void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
        vertexColor = aColor;
    }
    )";
    
    fragmentShaderSource = R"(
    #version 330 core
    in vec3 vertexColor;
    out vec4 FragColor;
    
    void main() {
        FragColor = vec4(vertexColor, 1.0);
    }
    )";
    
    // 图像显示着色器源码
    imageVertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec2 aTexCoord;
    
    out vec2 TexCoord;
    
    void main() {
        gl_Position = vec4(aPos, 0.0, 1.0);
        TexCoord = aTexCoord;
    }
    )";
    
    imageFragmentShaderSource = R"(
    #version 330 core
    in vec2 TexCoord;
    out vec4 FragColor;
    
    uniform sampler2D ourTexture;
    
    void main() {
        float gray = texture(ourTexture, TexCoord).r;
        FragColor = vec4(gray, gray, gray, 1.0);
    }
    )";
    
    // 坐标轴着色器源码
    axisVertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aColor;
    
    uniform mat4 view;
    uniform mat4 projection;
    
    out vec3 vertexColor;
    
    void main() {
        gl_Position = projection * view * vec4(aPos, 1.0);
        vertexColor = aColor;
    }
    )";
    
    axisFragmentShaderSource = R"(
    #version 330 core
    in vec3 vertexColor;
    out vec4 FragColor;
    
    void main() {
        FragColor = vec4(vertexColor, 1.0);
    }
    )";
}

OpenGLRenderer::~OpenGLRenderer() {
    cleanup();
}

bool OpenGLRenderer::initializeWithoutWindow() {
    // 注意：此函数假设已有有效的OpenGL上下文
    // 通常需要在调用此函数前创建一个隐藏窗口或离屏上下文
    
    // 设置 OpenGL 状态
    glEnable(GL_DEPTH_TEST);
    glPointSize(2.0f);
    glLineWidth(2.0f);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    // 编译和链接 3D 着色器程序
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    
    if (vertexShader == 0 || fragmentShader == 0) {
        std::cerr << "Failed to compile 3D shaders in initializeWithoutWindow" << std::endl;
        return false;
    }
    
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    // 检查链接状态
    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "3D Shader program linking failed in initializeWithoutWindow: " << infoLog << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }
    
    // 编译和链接图像着色器程序
    GLuint imageVertexShader = compileShader(GL_VERTEX_SHADER, imageVertexShaderSource);
    GLuint imageFragmentShader = compileShader(GL_FRAGMENT_SHADER, imageFragmentShaderSource);
    
    if (imageVertexShader == 0 || imageFragmentShader == 0) {
        std::cerr << "Failed to compile image shaders in initializeWithoutWindow" << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }
    
    imageShaderProgram = glCreateProgram();
    glAttachShader(imageShaderProgram, imageVertexShader);
    glAttachShader(imageShaderProgram, imageFragmentShader);
    glLinkProgram(imageShaderProgram);
    
    // 检查链接状态
    glGetProgramiv(imageShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(imageShaderProgram, 512, NULL, infoLog);
        std::cerr << "Image shader program linking failed in initializeWithoutWindow: " << infoLog << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteShader(imageVertexShader);
        glDeleteShader(imageFragmentShader);
        return false;
    }
    
    // 编译和链接坐标轴着色器程序
    GLuint axisVertexShader = compileShader(GL_VERTEX_SHADER, axisVertexShaderSource);
    GLuint axisFragmentShader = compileShader(GL_FRAGMENT_SHADER, axisFragmentShaderSource);
    
    if (axisVertexShader == 0 || axisFragmentShader == 0) {
        std::cerr << "Failed to compile axis shaders in initializeWithoutWindow" << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteShader(imageVertexShader);
        glDeleteShader(imageFragmentShader);
        return false;
    }
    
    axisShaderProgram = glCreateProgram();
    glAttachShader(axisShaderProgram, axisVertexShader);
    glAttachShader(axisShaderProgram, axisFragmentShader);
    glLinkProgram(axisShaderProgram);
    
    // 检查链接状态
    glGetProgramiv(axisShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(axisShaderProgram, 512, NULL, infoLog);
        std::cerr << "Axis shader program linking failed in initializeWithoutWindow: " << infoLog << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteShader(imageVertexShader);
        glDeleteShader(imageFragmentShader);
        glDeleteShader(axisVertexShader);
        glDeleteShader(axisFragmentShader);
        return false;
    }
    
    // 清理着色器对象
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(imageVertexShader);
    glDeleteShader(imageFragmentShader);
    glDeleteShader(axisVertexShader);
    glDeleteShader(axisFragmentShader);
    
    // 设置投影矩阵（使用默认的长宽比）
    projection = glm::perspective(
        glm::radians(45.0f),
        800.0f / 600.0f,  // 默认长宽比
        0.1f,
        100.0f
    );
    
    // 设置图像显示的顶点数据
    setupImageQuad();
    
    // 设置坐标轴数据
    setupAxisData();
    
    // 初始化摄像头位置
    updateCamera();
    
    std::cout << "OpenGL resources initialized without window" << std::endl;
    return true;
}

bool OpenGLRenderer::initialize() {
    // 初始化 GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    // 设置 OpenGL 版本和配置
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // 创建窗口
    window = glfwCreateWindow(800, 600, "FFT Image Processor - Press 1,2,3 to switch views | Drag in 3D mode to rotate", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    
    // 设置回调函数
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    
    // 初始化 GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return false;
    }
    
    // 设置 OpenGL 状态
    glEnable(GL_DEPTH_TEST);
    glPointSize(2.0f);
    glLineWidth(2.0f);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    // 编译和链接 3D 着色器程序
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    
    if (vertexShader == 0 || fragmentShader == 0) {
        std::cerr << "Failed to compile 3D shaders" << std::endl;
        return false;
    }
    
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    // 检查链接状态
    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "3D Shader program linking failed: " << infoLog << std::endl;
        return false;
    }
    
    // 编译和链接图像着色器程序
    GLuint imageVertexShader = compileShader(GL_VERTEX_SHADER, imageVertexShaderSource);
    GLuint imageFragmentShader = compileShader(GL_FRAGMENT_SHADER, imageFragmentShaderSource);
    
    if (imageVertexShader == 0 || imageFragmentShader == 0) {
        std::cerr << "Failed to compile image shaders" << std::endl;
        return false;
    }
    
    imageShaderProgram = glCreateProgram();
    glAttachShader(imageShaderProgram, imageVertexShader);
    glAttachShader(imageShaderProgram, imageFragmentShader);
    glLinkProgram(imageShaderProgram);
    
    // 检查链接状态
    glGetProgramiv(imageShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(imageShaderProgram, 512, NULL, infoLog);
        std::cerr << "Image shader program linking failed: " << infoLog << std::endl;
        return false;
    }
    
    // 编译和链接坐标轴着色器程序
    GLuint axisVertexShader = compileShader(GL_VERTEX_SHADER, axisVertexShaderSource);
    GLuint axisFragmentShader = compileShader(GL_FRAGMENT_SHADER, axisFragmentShaderSource);
    
    if (axisVertexShader == 0 || axisFragmentShader == 0) {
        std::cerr << "Failed to compile axis shaders" << std::endl;
        return false;
    }
    
    axisShaderProgram = glCreateProgram();
    glAttachShader(axisShaderProgram, axisVertexShader);
    glAttachShader(axisShaderProgram, axisFragmentShader);
    glLinkProgram(axisShaderProgram);
    
    // 检查链接状态
    glGetProgramiv(axisShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(axisShaderProgram, 512, NULL, infoLog);
        std::cerr << "Axis shader program linking failed: " << infoLog << std::endl;
        return false;
    }
    
    // 清理着色器对象
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(imageVertexShader);
    glDeleteShader(imageFragmentShader);
    glDeleteShader(axisVertexShader);
    glDeleteShader(axisFragmentShader);
    
    // 设置投影矩阵
    projection = glm::perspective(
        glm::radians(45.0f),
        800.0f / 600.0f,
        0.1f,
        100.0f
    );
    
    // 设置图像显示的顶点数据
    setupImageQuad();
    
    // 设置坐标轴数据
    setupAxisData();
    
    std::cout << "OpenGL Renderer initialized successfully" << std::endl;
    return true;
}

GLuint OpenGLRenderer::compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Shader compilation failed (" 
                  << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") 
                  << "): " << infoLog << std::endl;
        return 0;
    }
    
    return shader;
}

void OpenGLRenderer::setupImageQuad() {
    // 先清理已有资源
    cleanupImageQuad();
    
    float quadVertices[] = {
        // 位置      // 纹理坐标
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    
    unsigned int quadIndices[] = {
        0, 1, 2,
        0, 2, 3
    };
    
    glGenVertexArrays(1, &imageVAO);
    glGenBuffers(1, &imageVBO);
    glGenBuffers(1, &imageEBO);  // 使用成员变量
    
    glBindVertexArray(imageVAO);
    glBindBuffer(GL_ARRAY_BUFFER, imageVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, imageEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);
    
    // 位置属性
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 纹理坐标属性
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
}

void OpenGLRenderer::setupAxisData() {
    // 先清理已有资源
    cleanupAxisData();
    // 创建坐标轴数据：X轴(红色)、Y轴(绿色)、Z轴(蓝色)
    // 每条轴线加上箭头指示正方向
    std::vector<float> axisVertices;
    
    // X轴 (红色) - 从原点到正X方向
    // 主轴线
    axisVertices.insert(axisVertices.end(), {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f}); // 原点
    axisVertices.insert(axisVertices.end(), {2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f}); // X轴终点
    
    // X轴箭头
    axisVertices.insert(axisVertices.end(), {2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f}); // 箭头起点
    axisVertices.insert(axisVertices.end(), {1.8f, 0.1f, 0.0f, 1.0f, 0.0f, 0.0f}); // 箭头分支1
    axisVertices.insert(axisVertices.end(), {2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f}); // 箭头起点
    axisVertices.insert(axisVertices.end(), {1.8f, -0.1f, 0.0f, 1.0f, 0.0f, 0.0f}); // 箭头分支2
    
    // Y轴 (绿色) - 从原点到正Y方向
    // 主轴线
    axisVertices.insert(axisVertices.end(), {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f}); // 原点
    axisVertices.insert(axisVertices.end(), {0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f}); // Y轴终点
    
    // Y轴箭头
    axisVertices.insert(axisVertices.end(), {0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f}); // 箭头起点
    axisVertices.insert(axisVertices.end(), {0.1f, 1.8f, 0.0f, 0.0f, 1.0f, 0.0f}); // 箭头分支1
    axisVertices.insert(axisVertices.end(), {0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f}); // 箭头起点
    axisVertices.insert(axisVertices.end(), {-0.1f, 1.8f, 0.0f, 0.0f, 1.0f, 0.0f}); // 箭头分支2
    
    // Z轴 (蓝色) - 从原点到正Z方向
    // 主轴线
    axisVertices.insert(axisVertices.end(), {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f}); // 原点
    axisVertices.insert(axisVertices.end(), {0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f}); // Z轴终点
    
    // Z轴箭头
    axisVertices.insert(axisVertices.end(), {0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f}); // 箭头起点
    axisVertices.insert(axisVertices.end(), {0.1f, 0.0f, 1.8f, 0.0f, 0.0f, 1.0f}); // 箭头分支1
    axisVertices.insert(axisVertices.end(), {0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 1.0f}); // 箭头起点
    axisVertices.insert(axisVertices.end(), {-0.1f, 0.0f, 1.8f, 0.0f, 0.0f, 1.0f}); // 箭头分支2
    
    glGenVertexArrays(1, &axisVAO);
    glGenBuffers(1, &axisVBO);
    
    glBindVertexArray(axisVAO);
    glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
    glBufferData(GL_ARRAY_BUFFER, axisVertices.size() * sizeof(float), axisVertices.data(), GL_STATIC_DRAW);
    
    // 位置属性 (x, y, z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 颜色属性 (r, g, b)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
}

void OpenGLRenderer::setupVertexData(const std::vector<float>& vertices) {
    // 先清理已有资源
    cleanupVertexData();
    
    // 计算摄像头中心点
    calculateCameraCenter(vertices);
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 颜色属性
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
    
    // 更新摄像头
    updateCamera();
}

void OpenGLRenderer::calculateCameraCenter(const std::vector<float>& vertices) {
    if (vertices.empty()) {
        cameraCenter = glm::vec3(0.0f, 0.0f, 0.0f);
        return;
    }
    
    // 提取所有Z值
    std::vector<float> zValues;
    for (size_t i = 2; i < vertices.size(); i += 6) { // 每6个float一个顶点，Z是第3个
        zValues.push_back(vertices[i]);
    }
    
    if (zValues.empty()) {
        cameraCenter = glm::vec3(0.0f, 0.0f, 0.0f);
        return;
    }
    
    // 排序找中位数
    std::sort(zValues.begin(), zValues.end());
    float medianZ = zValues[zValues.size() / 2];
    
    // 设置摄像头中心点，X和Y保持为0，Z为中位数
    cameraCenter = glm::vec3(0.0f, 0.0f, medianZ);
    
    std::cout << "Camera center set to: (" << cameraCenter.x << ", " 
              << cameraCenter.y << ", " << cameraCenter.z << ")" << std::endl;
}

void OpenGLRenderer::updateCamera() {
    // 将球坐标转换为笛卡尔坐标
    float x = cameraCenter.x + cameraRadius * sin(glm::radians(cameraPhi)) * cos(glm::radians(cameraTheta));
    float y = cameraCenter.y + cameraRadius * sin(glm::radians(cameraPhi)) * sin(glm::radians(cameraTheta));
    float z = cameraCenter.z + cameraRadius * cos(glm::radians(cameraPhi));
    
    glm::vec3 cameraPos(x, y, z);
    glm::vec3 up(0.0f, 0.0f, 1.0f); // Z轴向上
    
    view = glm::lookAt(cameraPos, cameraCenter, up);
}

void OpenGLRenderer::setupImageTexture(const std::vector<std::vector<double>>& imageData, int width, int height) {
    // 先清理已有纹理
    cleanupTexture();
    
    // 将double图像数据转换为unsigned char
    std::vector<unsigned char> textureData(width * height);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            textureData[y * width + x] = (unsigned char)std::max(0.0, std::min(255.0, imageData[y][x]));
        }
    }
    
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, textureData.data());
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLRenderer::updateImageTexture(const std::vector<std::vector<double>>& imageData, int width, int height) {
    std::vector<unsigned char> textureData(width * height);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            textureData[y * width + x] = (unsigned char)std::max(0.0, std::min(255.0, imageData[y][x]));
        }
    }
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, textureData.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLRenderer::render(int pointCount, 
                           const std::vector<std::vector<double>>& originalImage, 
                           const std::vector<std::vector<double>>& reconstructedImage, 
                           int width, int height) {
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        switch (displayMode) {
            case 0:
                // 显示原始图像
                renderImage(originalImage, width, height);
                break;
            case 1:
                // 显示3D频域
                render3DFrequency(pointCount);
                renderAxes(); // 同时显示坐标轴
                break;
            case 2:
                // 显示重建图像
                renderImage(reconstructedImage, width, height);
                break;
            default:
                renderImage(originalImage, width, height);
                break;
        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void OpenGLRenderer::renderImage(const std::vector<std::vector<double>>& imageData, int width, int height) {
    glDisable(GL_DEPTH_TEST);
    
    updateImageTexture(imageData, width, height);
    
    glUseProgram(imageShaderProgram);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glBindVertexArray(imageVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    
    glEnable(GL_DEPTH_TEST);
}

void OpenGLRenderer::render3DFrequency(int pointCount) {
    glUseProgram(shaderProgram);
    
    // 移除自动旋转，使用静态模型矩阵
    glm::mat4 model = glm::mat4(1.0f); // 单位矩阵，无变换
    
    // 设置 uniform 变量
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, pointCount);
    glBindVertexArray(0);
    glUseProgram(0);
}

void OpenGLRenderer::renderAxes() {
    glUseProgram(axisShaderProgram);
    
    // 设置 uniform 变量（坐标轴不需要模型变换）
    GLint viewLoc = glGetUniformLocation(axisShaderProgram, "view");
    GLint projLoc = glGetUniformLocation(axisShaderProgram, "projection");
    
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    
    glBindVertexArray(axisVAO);
    glDrawArrays(GL_LINES, 0, 16); // 16个顶点：3条主轴线 + 6条箭头线
    glBindVertexArray(0);
    glUseProgram(0);
}

void OpenGLRenderer::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode; // 标记未使用的参数
    (void)mods;     // 标记未使用的参数
    
    OpenGLRenderer* renderer = static_cast<OpenGLRenderer*>(glfwGetWindowUserPointer(window));
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_1:
                renderer->displayMode = 0;
                std::cout << "Displaying: Original Grayscale Image" << std::endl;
                break;
            case GLFW_KEY_2:
                renderer->displayMode = 1;
                std::cout << "Displaying: 3D Frequency Domain with Axes" << std::endl;
                break;
            case GLFW_KEY_3:
                renderer->displayMode = 2;
                std::cout << "Displaying: Reconstructed Image" << std::endl;
                break;
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                break;
        }
    }
}

void OpenGLRenderer::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    (void)mods; // 标记未使用的参数
    
    OpenGLRenderer* renderer = static_cast<OpenGLRenderer*>(glfwGetWindowUserPointer(window));
    
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            renderer->mousePressed = true;
            glfwGetCursorPos(window, &renderer->lastMouseX, &renderer->lastMouseY);
        } else if (action == GLFW_RELEASE) {
            renderer->mousePressed = false;
        }
    }
}

void OpenGLRenderer::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    OpenGLRenderer* renderer = static_cast<OpenGLRenderer*>(glfwGetWindowUserPointer(window));
    
    if (renderer->mousePressed && renderer->displayMode == 1) { // 只在3D模式下响应鼠标
        double deltaX = xpos - renderer->lastMouseX;
        double deltaY = ypos - renderer->lastMouseY;
        
        // 调整旋转敏感度
        float sensitivity = 0.5f;
        
        // 更新球坐标 - 反转方向使拖拽更直觉
        renderer->cameraTheta -= deltaX * sensitivity;  // 反转水平方向
        renderer->cameraPhi -= deltaY * sensitivity;    // 反转垂直方向
        
        // 限制phi角度范围，避免翻转
        renderer->cameraPhi = std::max(10.0f, std::min(170.0f, renderer->cameraPhi));
        
        // 更新摄像头位置
        renderer->updateCamera();
        
        renderer->lastMouseX = xpos;
        renderer->lastMouseY = ypos;
    }
}

void OpenGLRenderer::cleanup() {
    cleanupVertexData();
    cleanupImageQuad();
    cleanupAxisData();
    cleanupTexture();
    
    if (shaderProgram != 0) {
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
    }
    if (imageShaderProgram != 0) {
        glDeleteProgram(imageShaderProgram);
        imageShaderProgram = 0;
    }
    if (axisShaderProgram != 0) {
        glDeleteProgram(axisShaderProgram);
        axisShaderProgram = 0;
    }
    
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    
    glfwTerminate();
}

bool OpenGLRenderer::shouldClose() {
    return window ? glfwWindowShouldClose(window) : true;
}

void OpenGLRenderer::handleMouseDrag(float deltaX, float deltaY) {
    float sensitivity = 0.5f;
    cameraTheta -= deltaX * sensitivity;
    cameraPhi -= deltaY * sensitivity;
    cameraPhi = std::max(10.0f, std::min(170.0f, cameraPhi));
    updateCamera();
}

void OpenGLRenderer::handleMouseWheel(float delta) {
    cameraRadius -= delta * 0.5f;
    cameraRadius = std::max(1.0f, std::min(20.0f, cameraRadius));
    updateCamera();
}

void OpenGLRenderer::cleanupVertexData() {
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
    if (VBO != 0) {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }
}

void OpenGLRenderer::cleanupImageQuad() {
    if (imageVAO != 0) {
        glDeleteVertexArrays(1, &imageVAO);
        imageVAO = 0;
    }
    if (imageVBO != 0) {
        glDeleteBuffers(1, &imageVBO);
        imageVBO = 0;
    }
    if (imageEBO != 0) {
        glDeleteBuffers(1, &imageEBO);
        imageEBO = 0;
    }
}

void OpenGLRenderer::cleanupAxisData() {
    if (axisVAO != 0) {
        glDeleteVertexArrays(1, &axisVAO);
        axisVAO = 0;
    }
    if (axisVBO != 0) {
        glDeleteBuffers(1, &axisVBO);
        axisVBO = 0;
    }
}

void OpenGLRenderer::cleanupTexture() {
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }
}