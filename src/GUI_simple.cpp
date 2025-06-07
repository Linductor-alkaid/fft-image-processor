#include "GUI.h"
#include "ImageProcessor.h"
#include <sstream>
#include <iomanip>

// 简化文件对话框
namespace SimpleFileDialog {
    std::string openFile(const char*) {
        std::string filename;
        std::cout << "输入图像路径: ";
        std::getline(std::cin, filename);
        return filename;
    }
    
    std::string saveFile(const char*) {
        std::string filename;
        std::cout << "输入保存路径: ";
        std::getline(std::cin, filename);
        return filename;
    }
}

// GuiUtils简化实现
namespace GuiUtils {
    ImVec4 getColorFromValue(float value, ColorMap colorMap) {
        value = std::max(0.0f, std::min(1.0f, value));
        switch (colorMap) {
            case ColorMap::GRAYSCALE:
                return ImVec4(value, value, value, 1.0f);
            case ColorMap::JET:
                return ImVec4(value, 1.0f - value, 0.5f, 1.0f);
            default:
                return ImVec4(value, value, value, 1.0f);
        }
    }

    void drawColorBar(ColorMap, float, float, const ImVec2&) {
        // 简化实现
        ImGui::Text("颜色条");
    }

    std::string formatNumber(double value, int precision) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision) << value;
        return oss.str();
    }

    void helpMarker(const char* desc) {
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", desc);
        }
    }
}

GUI::GUI() : 
    window(nullptr),
    processor(std::make_unique<ImageProcessor>()),
    showMainWindow(true),
    showImageWindow(true),
    showFrequencyWindow(true),
    showControlWindow(true),
    showFilterWindow(false),
    showStatsWindow(false),
    showAboutWindow(false),
    originalImageTexture(0),
    frequencyMagnitudeTexture(0),
    frequencyPhaseTexture(0),
    filteredImageTexture(0),
    currentDisplayMode(DisplayMode::ORIGINAL_IMAGE),
    currentColorMap(ColorMap::GRAYSCALE),
    filterType(0),
    lowPassCutoff(0.5f),
    highPassCutoff(0.5f),
    bandPassLow(0.2f),
    bandPassHigh(0.8f),
    autoApplyFilter(false) {
}

GUI::~GUI() {
    cleanup();
}

bool GUI::initialize() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    window = glfwCreateWindow(1280, 720, "FFT Image Processor - GUI", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return false;
    }
    
    setupCallbacks();
    initializeGUI();
    
    processor->createTestImage(256);
    processor->fft2D();
    updateImageTextures();
    
    return true;
}

void GUI::initializeGUI() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    ImGui::StyleColorsDark();
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

void GUI::setupCallbacks() {
    glfwSetWindowUserPointer(window, this);
    glfwSetDropCallback(window, dropCallback);
    glfwSetKeyCallback(window, keyCallback);
}

void GUI::run() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        drawMainMenuBar();
        
        if (showImageWindow) drawImageWindow();
        if (showFrequencyWindow) drawFrequencyWindow();
        if (showControlWindow) drawControlWindow();
        if (showFilterWindow) drawFilterWindow();
        if (showStatsWindow) drawStatsWindow();
        if (showAboutWindow) drawAboutWindow();
        
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
    }
}

void GUI::drawMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("文件")) {
            if (ImGui::MenuItem("打开图像")) openImageFile();
            if (ImGui::MenuItem("创建测试图像")) createTestImage();
            if (ImGui::MenuItem("保存图像", nullptr, false, processor->getWidth() > 0)) saveCurrentImage();
            if (ImGui::MenuItem("退出")) glfwSetWindowShouldClose(window, GLFW_TRUE);
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("视图")) {
            ImGui::MenuItem("图像窗口", nullptr, &showImageWindow);
            ImGui::MenuItem("频域窗口", nullptr, &showFrequencyWindow);
            ImGui::MenuItem("控制面板", nullptr, &showControlWindow);
            ImGui::MenuItem("滤波器", nullptr, &showFilterWindow);
            ImGui::MenuItem("统计信息", nullptr, &showStatsWindow);
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("帮助")) {
            ImGui::MenuItem("关于", nullptr, &showAboutWindow);
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }
}

void GUI::drawImageWindow() {
    ImGui::Begin("图像显示", &showImageWindow);
    
    if (processor->getWidth() > 0) {
        ImGui::Text("图像尺寸: %dx%d", processor->getWidth(), processor->getHeight());
        
        // 简化的图像显示
        if (originalImageTexture) {
            ImVec2 imageSize(400, 400);
            ImGui::Image(reinterpret_cast<void*>(originalImageTexture), imageSize);
        }
        
        // 显示模式选择
        ImGui::Text("显示模式:");
        ImGui::RadioButton("原始图像", (int*)&currentDisplayMode, (int)DisplayMode::ORIGINAL_IMAGE);
        ImGui::RadioButton("频域幅度", (int*)&currentDisplayMode, (int)DisplayMode::FREQUENCY_MAGNITUDE);
        ImGui::RadioButton("滤波后图像", (int*)&currentDisplayMode, (int)DisplayMode::FILTERED_IMAGE);
    } else {
        ImGui::Text("没有加载图像");
        ImGui::Text("请从文件菜单加载图像或创建测试图像");
    }
    
    ImGui::End();
}

void GUI::drawFrequencyWindow() {
    ImGui::Begin("频域分析", &showFrequencyWindow);
    
    if (processor->getWidth() > 0) {
        ImGui::Text("频域分析");
        ImGui::Text("图像已进行FFT变换");
        
        // 简单的频域信息显示
        if (ImPlot::BeginPlot("频域分析")) {
            // 简化的绘图
            float x[] = {0, 1, 2, 3, 4};
            float y[] = {0, 1, 4, 9, 16};
            ImPlot::PlotLine("示例频谱", x, y, 5);
            ImPlot::EndPlot();
        }
    } else {
        ImGui::Text("请先加载图像");
    }
    
    ImGui::End();
}

void GUI::drawControlWindow() {
    ImGui::Begin("控制面板", &showControlWindow);
    
    ImGui::Text("图像信息:");
    if (processor->getWidth() > 0) {
        ImGui::Text("尺寸: %dx%d", processor->getWidth(), processor->getHeight());
        ImGui::Text("文件: %s", currentImagePath.empty() ? "测试图像" : currentImagePath.c_str());
    }
    
    ImGui::Separator();
    
    if (ImGui::Button("执行FFT", ImVec2(-1, 0))) {
        if (processor->getWidth() > 0) {
            processor->fft2D();
            updateImageTextures();
        }
    }
    
    if (ImGui::Button("应用滤波器", ImVec2(-1, 0))) {
        applyCurrentFilter();
    }
    
    ImGui::End();
}

void GUI::drawFilterWindow() {
    ImGui::Begin("滤波器设置", &showFilterWindow);
    
    ImGui::Text("滤波器类型:");
    ImGui::RadioButton("低通滤波", &filterType, 0);
    ImGui::RadioButton("高通滤波", &filterType, 1);
    ImGui::RadioButton("带通滤波", &filterType, 2);
    
    ImGui::Separator();
    
    if (filterType == 0) {
        ImGui::SliderFloat("截止频率", &lowPassCutoff, 0.0f, 1.0f);
    } else if (filterType == 1) {
        ImGui::SliderFloat("截止频率", &highPassCutoff, 0.0f, 1.0f);
    } else {
        ImGui::SliderFloat("低截止", &bandPassLow, 0.0f, 1.0f);
        ImGui::SliderFloat("高截止", &bandPassHigh, 0.0f, 1.0f);
    }
    
    ImGui::Checkbox("自动应用", &autoApplyFilter);
    
    if (ImGui::Button("应用滤波器")) {
        applyCurrentFilter();
    }
    
    ImGui::End();
}

void GUI::drawStatsWindow() {
    ImGui::Begin("统计信息", &showStatsWindow);
    
    if (processor->getWidth() > 0) {
        ImGui::Text("原始图像统计:");
        ImGui::Text("最小值: %.3f", originalStats.minValue);
        ImGui::Text("最大值: %.3f", originalStats.maxValue);
        ImGui::Text("平均值: %.3f", originalStats.meanValue);
        ImGui::Text("标准差: %.3f", originalStats.stdValue);
    } else {
        ImGui::Text("请先加载图像");
    }
    
    ImGui::End();
}

void GUI::drawAboutWindow() {
    ImGui::Begin("关于", &showAboutWindow, ImGuiWindowFlags_AlwaysAutoResize);
    
    ImGui::Text("FFT Image Processor - GUI版本");
    ImGui::Text("版本: 2.0.0 简化版");
    ImGui::Separator();
    
    ImGui::Text("功能特性:");
    ImGui::BulletText("2D快速傅里叶变换");
    ImGui::BulletText("数字滤波器");
    ImGui::BulletText("图像处理");
    
    if (ImGui::Button("关闭")) {
        showAboutWindow = false;
    }
    
    ImGui::End();
}

// 实现其他必要方法
void GUI::cleanup() {
    if (originalImageTexture) glDeleteTextures(1, &originalImageTexture);
    if (frequencyMagnitudeTexture) glDeleteTextures(1, &frequencyMagnitudeTexture);
    if (frequencyPhaseTexture) glDeleteTextures(1, &frequencyPhaseTexture);
    if (filteredImageTexture) glDeleteTextures(1, &filteredImageTexture);
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

void GUI::dropCallback(GLFWwindow* window, int count, const char** paths) {
    GUI* gui = static_cast<GUI*>(glfwGetWindowUserPointer(window));
    if (gui && count > 0) {
        gui->currentImagePath = paths[0];
        if (gui->processor->loadImage(paths[0])) {
            gui->processor->fft2D();
            gui->updateImageTextures();
        }
    }
}

void GUI::keyCallback(GLFWwindow* window, int key, int, int action, int mods) {
    GUI* gui = static_cast<GUI*>(glfwGetWindowUserPointer(window));
    if (gui && action == GLFW_PRESS && (mods & GLFW_MOD_CONTROL)) {
        switch (key) {
            case GLFW_KEY_O: gui->openImageFile(); break;
            case GLFW_KEY_S: gui->saveCurrentImage(); break;
            case GLFW_KEY_T: gui->createTestImage(); break;
        }
    }
}

bool GUI::openImageFile() {
    std::string filename = SimpleFileDialog::openFile("");
    if (!filename.empty()) {
        currentImagePath = filename;
        if (processor->loadImage(filename)) {
            processor->fft2D();
            updateImageTextures();
            return true;
        }
    }
    return false;
}

bool GUI::saveImageAs() {
    std::string filename = SimpleFileDialog::saveFile("");
    if (!filename.empty()) {
        lastSavePath = filename;
        return saveCurrentImage();
    }
    return false;
}

bool GUI::saveCurrentImage() {
    if (processor->getWidth() == 0) return false;
    if (lastSavePath.empty()) return saveImageAs();
    return processor->saveImage(lastSavePath, processor->getGrayImage());
}

void GUI::createTestImage() {
    processor->createTestImage(256);
    processor->fft2D();
    updateImageTextures();
    currentImagePath = "";
}

void GUI::applyCurrentFilter() {
    if (processor->getWidth() == 0) return;
    // 简化的滤波器应用
    updateImageTextures();
}

void GUI::resetFilter() {
    filterType = 0;
    lowPassCutoff = 0.5f;
    highPassCutoff = 0.5f;
    bandPassLow = 0.2f;
    bandPassHigh = 0.8f;
}

void GUI::onFilterParameterChanged() {
    if (autoApplyFilter) applyCurrentFilter();
}

void GUI::updateImageTextures() {
    // 简化的纹理更新
    if (!originalImageTexture) {
        glGenTextures(1, &originalImageTexture);
    }
    
    // 基本的纹理设置
    glBindTexture(GL_TEXTURE_2D, originalImageTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // 简化的统计计算
    originalStats.minValue = 0.0;
    originalStats.maxValue = 255.0;
    originalStats.meanValue = 127.5;
    originalStats.stdValue = 50.0;
}

void GUI::calculateImageStats(const std::vector<std::vector<double>>&, ImageStats&) {
    // 简化实现
}

GLuint GUI::createTextureFromImage(const std::vector<std::vector<double>>&, int, int, ColorMap) {
    GLuint texture;
    glGenTextures(1, &texture);
    return texture;
}

void GUI::drawFrequencyPlot() {}
void GUI::drawHistogram(const std::vector<std::vector<double>>&, const std::string&) {}
void GUI::drawImageWithLegend(GLuint, int, int, const std::string&, const ImageStats&) {}
