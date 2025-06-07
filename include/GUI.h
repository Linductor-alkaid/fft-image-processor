#ifndef GUI_H
#define GUI_H

// 标准库
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <algorithm>
#include <cmath>

#include "ImageProcessor.h"
#include "OpenGLRenderer.h"

// OpenGL相关
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// 前向声明
class ImageProcessor;
class Complex;

// ImGui相关
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "implot/implot.h"

// 简化的文件对话框
namespace SimpleFileDialog {
    std::string openFile(const char* filter = nullptr);
    std::string saveFile(const char* filter = nullptr);
}

// 颜色映射类型
enum class ColorMap {
    GRAYSCALE,
    JET,
    HOT,
    COOL,
    VIRIDIS
};

// 显示模式
enum class DisplayMode {
    ORIGINAL_IMAGE,
    FREQUENCY_MAGNITUDE,
    FREQUENCY_PHASE,
    FILTERED_IMAGE,
    COMPARISON_VIEW
};

class GUI {
private:
    bool chinese_font_loaded;
    bool loadChineseFont();
    const char* getDisplayText(const char* text);
    GLFWwindow* window;
    std::unique_ptr<ImageProcessor> processor;
    
    // GUI状态变量
    bool showMainWindow;
    bool showImageWindow;
    bool showFrequencyWindow;
    bool showControlWindow;
    bool showFilterWindow;
    bool showStatsWindow;
    bool showAboutWindow;
    
    // 图像显示相关
    GLuint originalImageTexture;
    GLuint frequencyMagnitudeTexture;
    GLuint frequencyPhaseTexture;
    GLuint filteredImageTexture;
    DisplayMode currentDisplayMode;
    ColorMap currentColorMap;
    
    // 滤波器参数
    int filterType;
    float lowPassCutoff;
    float highPassCutoff;
    float bandPassLow;
    float bandPassHigh;
    bool autoApplyFilter;
    
    // 图像统计
    struct ImageStats {
        double mse;
        double psnr;
        double ssim;
        double meanValue;
        double stdValue;
        double minValue;
        double maxValue;
    } originalStats, filteredStats;
    
    // 文件路径
    std::string currentImagePath;
    std::string lastSavePath;

    // 3d图形相关
    std::unique_ptr<OpenGLRenderer> openglRenderer;
    bool show3DWindow;
    GLuint framebufferID;
    GLuint colorTextureID;
    GLuint depthTextureID;
    int renderWidth, renderHeight;
    
    // 私有方法
    void initializeGUI();
    void setupCallbacks();
    GLuint createTextureFromImage(const std::vector<std::vector<double>>& image, 
                                  int width, int height, ColorMap colorMap);
    void updateImageTextures();
    void calculateImageStats(const std::vector<std::vector<double>>& image, ImageStats& stats);
    void drawImageWithLegend(GLuint texture, int width, int height, 
                           const std::string& title, const ImageStats& stats);
    void drawFrequencyPlot();
    void drawHistogram(const std::vector<std::vector<double>>& image, const std::string& title);
    
    // GUI窗口绘制方法
    void drawMainMenuBar();
    void drawImageWindow();
    void drawFrequencyWindow();
    void drawControlWindow();
    void drawFilterWindow();
    void drawStatsWindow();
    void drawAboutWindow();
    
    // 文件操作
    bool openImageFile();
    bool saveCurrentImage();
    bool saveImageAs();
    void createTestImage();
    
    // 滤波器操作
    void applyCurrentFilter();
    void resetFilter();
    void onFilterParameterChanged();

    // 3D渲染
    void init3DRenderer();
    void setup3DFramebuffer(int width, int height);
    void render3DToTexture();
    void draw3DVisualizationWindow();
    void cleanup3DRenderer();
    
public:
    GUI();
    ~GUI();
    
    bool initialize();
    void run();
    void cleanup();
    
    // 静态回调函数
    static void dropCallback(GLFWwindow* window, int count, const char** paths);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};

// 工具函数
namespace GuiUtils {
    ImVec4 getColorFromValue(float value, ColorMap colorMap);
    void drawColorBar(ColorMap colorMap, float minVal, float maxVal, const ImVec2& size);
    std::string formatNumber(double value, int precision = 3);
    void helpMarker(const char* desc);
}

#endif // GUI_H
