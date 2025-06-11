#include "GUI.h"
#include <sstream>
#include <iomanip>
#include <thread>
#include <future>
#include <unordered_map>
#include <filesystem>

// 添加 nativefiledialog 支持
#ifdef HAS_NATIVEFILEDIALOG
    #include "nfd.h"
#endif

// 平台特定的头文件
#ifdef HAS_WINDOWS_DIALOGS
    #include <windows.h>
    #include <commdlg.h>
#endif

#ifdef HAS_GTK3
    #include <gtk/gtk.h>
#endif

// 文本映射系统 - 解决中文显示问题
class TextManager {
private:
    static std::unordered_map<std::string, std::string> text_map;
    static bool chinese_supported;
    
public:
    static void initializeTextMap() {
        text_map = {
            {"文件", "File"},
            {"视图", "View"}, 
            {"工具", "Tools"},
            {"帮助", "Help"},
            {"图像显示", "Image Display"},
            {"频域分析", "Frequency Analysis"},
            {"滤波器设置", "Filter Settings"},
            {"统计信息", "Statistics"},
            {"控制面板", "Control Panel"},
            {"关于", "About"},
            {"打开图像", "Open Image"},
            {"创建测试图像", "Create Test Image"},
            {"保存图像", "Save Image"},
            {"另存为...", "Save As..."},
            {"退出", "Exit"},
            {"原始图像", "Original Image"},
            {"频域幅度", "Frequency Magnitude"},
            {"频域相位", "Frequency Phase"}, 
            {"滤波后图像", "Filtered Image"},
            {"对比视图", "Comparison View"},
            {"显示模式", "Display Mode"},
            {"颜色映射", "Color Map"},
            {"灰度", "Grayscale"},
            {"热度图", "Hot"},
            {"冷色图", "Cool"},
            {"低通滤波", "Low Pass Filter"},
            {"高通滤波", "High Pass Filter"},
            {"带通滤波", "Band Pass Filter"},
            {"滤波器类型", "Filter Type"},
            {"截止频率", "Cutoff Frequency"},
            {"低截止频率", "Low Cutoff"},
            {"高截止频率", "High Cutoff"},
            {"自动应用", "Auto Apply"},
            {"手动应用", "Manual Apply"},
            {"应用滤波器", "Apply Filter"},
            {"重置参数", "Reset Parameters"},
            {"重置滤波器", "Reset Filter"},
            {"执行FFT", "Execute FFT"},
            {"重置视图", "Reset View"},
            {"快速操作", "Quick Actions"},
            {"图像信息", "Image Info"},
            {"显示控制", "Display Control"},
            {"尺寸", "Size"},
            {"通道数", "Channels"},
            {"文件", "File"},
            {"测试图像", "Test Image"},
            {"未加载图像", "No Image Loaded"},
            {"没有加载图像", "No Image Loaded"},
            {"请从文件菜单加载图像或创建测试图像", "Please load image from File menu or create test image"},
            {"创建测试图像", "Create Test Image"},
            {"最小值", "Min Value"},
            {"最大值", "Max Value"},
            {"平均值", "Mean Value"},
            {"标准差", "Std Dev"},
            {"统计信息", "Statistics"},
            {"原始图像统计", "Original Image Stats"},
            {"滤波后图像统计", "Filtered Image Stats"},
            {"质量指标说明", "Quality Metrics Description"},
            {"均方误差，越小越好", "Mean Square Error, smaller is better"},
            {"峰值信噪比，越大越好", "Peak Signal-to-Noise Ratio, larger is better"},
            {"结构相似性，接近1最好", "Structural Similarity, closer to 1 is better"},
            {"请先加载图像", "Please load image first"},
            {"频域分析", "Frequency Analysis"},
            {"图像已进行FFT变换", "Image has been FFT transformed"},
            {"幅度分布", "Magnitude Distribution"},
            {"频域信息", "Frequency Domain Info"},
            {"FFT变换已完成", "FFT transform completed"},
            {"频域数据可用于滤波处理", "Frequency data available for filtering"},
            {"重新计算FFT", "Recalculate FFT"},
            {"请先加载图像以查看频域分析", "Please load image first to view frequency analysis"},
            {"像素值分布", "Pixel Value Distribution"},
            {"版本", "Version"},
            {"功能特性", "Features"},
            {"技术栈", "Tech Stack"},
            {"快捷键", "Shortcuts"},
            {"使用提示", "Usage Tips"},
            {"现代化用户界面", "Modern User Interface"},
            {"专业数据可视化", "Professional Data Visualization"},
            {"高性能图形渲染", "High Performance Graphics Rendering"},
            {"跨平台窗口管理", "Cross-platform Window Management"},
            {"轻量级图像处理", "Lightweight Image Processing"},
            {"打开图像文件", "Open Image File"},
            {"保存当前图像", "Save Current Image"},
            {"直接拖拽图像文件到窗口", "Drag image files directly to window"},
            {"等常见格式", "and other common formats"},
            {"图像会自动调整为2的幂次尺寸", "Images will be automatically resized to power-of-2 dimensions"},
            {"可实时调整滤波器参数查看效果", "Real-time filter parameter adjustment"},
            {"所有窗口都可独立移动和调整", "All windows can be moved and resized independently"},
            {"关闭", "Close"},
            {"颜色条", "Color Bar"},
            {"预览效果", "Preview Effect"},
            {"原图 → 滤波器 → 结果图", "Original → Filter → Result"},
            {"可在图像窗口中切换查看效果", "Switch views in image window to see effects"},
            {"当前设置", "Current Setting"},
            {"保留", "Keep"},
            {"的低频成分", " low frequency components"},
            {"移除", "Remove"},
            {"频率范围", "Frequency Range"},
            {"警告: 低截止频率应小于高截止频率", "Warning: Low cutoff should be less than high cutoff"},
            {"启用后，参数改变时自动应用滤波器", "When enabled, filter is applied automatically when parameters change"},
            {"图像尺寸", "Image Size"},
            {"像素总数", "Total Pixels"},
            {"数据状态: 已加载", "Data Status: Loaded"},
            {"适应窗口", "Fit Window"}
        };
    }
    
    static const char* getText(const char* chinese_text) {
        if (chinese_supported) {
            return chinese_text;
        }
        
        auto it = text_map.find(chinese_text);
        return (it != text_map.end()) ? it->second.c_str() : chinese_text;
    }
    
    static void setChineseSupport(bool supported) {
        chinese_supported = supported;
    }
    
    static bool isChineseSupported() {
        return chinese_supported;
    }
};

// 静态成员定义
std::unordered_map<std::string, std::string> TextManager::text_map;
bool TextManager::chinese_supported = false;

// 便利宏
#define T(text) TextManager::getText(text)

// 文件对话框 
namespace SimpleFileDialog {
    std::string openFile(const char*) {
#ifdef HAS_NATIVEFILEDIALOG
        nfdchar_t *outPath = NULL;
        
        // 使用你的 API 版本：NFD_OpenDialog(filterList, defaultPath, outPath)
        nfdresult_t result = NFD_OpenDialog("png,jpg,jpeg,bmp,tiff", NULL, &outPath);
        
        if (result == NFD_OKAY) {
            std::string filepath(outPath);
            free(outPath);  // 释放内存
            std::cout << "✓ 选择文件: " << filepath << std::endl;
            return filepath;
        } else if (result == NFD_CANCEL) {
            std::cout << "用户取消选择文件" << std::endl;
        } else {
            std::cerr << "文件对话框错误: " << NFD_GetError() << std::endl;
        }
        return "";
#else
        // 终端输入回退
        std::string filename;
        std::cout << "\n=== 打开图像文件 ===" << std::endl;
        std::cout << "请输入图像文件的完整路径: ";
        std::cout.flush();
        
        std::cin.clear();
        if (std::cin.peek() == '\n') {
            std::cin.ignore();
        }
        
        std::getline(std::cin, filename);
        
        if (!filename.empty()) {
            filename.erase(0, filename.find_first_not_of(" \t\"'"));
            filename.erase(filename.find_last_not_of(" \t\"'") + 1);
        }
        
        if (!filename.empty() && std::filesystem::exists(filename)) {
            std::cout << "✓ 文件找到: " << filename << std::endl;
        } else if (!filename.empty()) {
            std::cout << "✗ 文件不存在: " << filename << std::endl;
            filename.clear();
        } else {
            std::cout << "操作已取消" << std::endl;
        }
        
        return filename;
#endif
    }
    
    std::string saveFile(const char*) {
#ifdef HAS_NATIVEFILEDIALOG
        nfdchar_t *outPath = NULL;
        
        // 使用你的 API 版本：NFD_SaveDialog(filterList, defaultPath, outPath)
        nfdresult_t result = NFD_SaveDialog("png", NULL, &outPath);
        
        if (result == NFD_OKAY) {
            std::string filepath(outPath);
            free(outPath);  // 释放内存
            std::cout << "✓ 选择保存路径: " << filepath << std::endl;
            return filepath;
        } else if (result == NFD_CANCEL) {
            std::cout << "用户取消保存" << std::endl;
        } else {
            std::cerr << "保存对话框错误: " << NFD_GetError() << std::endl;
        }
        return "";
#else
        // 终端输入回退
        std::string filename;
        std::cout << "\n=== 保存图像文件 ===" << std::endl;
        std::cout << "请输入保存路径 (例: /home/user/output.png): ";
        std::cout.flush();
        
        std::cin.clear();
        if (std::cin.peek() == '\n') {
            std::cin.ignore();
        }
        
        std::getline(std::cin, filename);
        
        if (!filename.empty()) {
            filename.erase(0, filename.find_first_not_of(" \t\"'"));
            filename.erase(filename.find_last_not_of(" \t\"'") + 1);
        }
        
        if (!filename.empty() && filename.find('.') == std::string::npos) {
            filename += ".png";
            std::cout << "自动添加扩展名: " << filename << std::endl;
        }
        
        if (!filename.empty()) {
            std::cout << "✓ 将保存到: " << filename << std::endl;
        } else {
            std::cout << "操作已取消" << std::endl;
        }
        
        return filename;
#endif
    }
}

// 清理NFD（在GUI清理时调用）
void cleanupFileDialogs() {
#ifdef HAS_NATIVEFILEDIALOG
    // 旧版本 nativefiledialog 不需要显式清理
    std::cout << "nativefiledialog 已完成" << std::endl;
#endif
}

// GuiUtils完整实现
namespace GuiUtils {
    ImVec4 getColorFromValue(float value, ColorMap colorMap) {
        value = std::max(0.0f, std::min(1.0f, value));
        
        switch (colorMap) {
            case ColorMap::GRAYSCALE:
                return ImVec4(value, value, value, 1.0f);
                
            case ColorMap::JET: {
                float r, g, b;
                if (value < 0.125f) {
                    r = 0.0f;
                    g = 0.0f;
                    b = 0.5f + 4.0f * value;
                } else if (value < 0.375f) {
                    r = 0.0f;
                    g = 4.0f * value - 0.5f;
                    b = 1.0f;
                } else if (value < 0.625f) {
                    r = 4.0f * value - 1.5f;
                    g = 1.0f;
                    b = -4.0f * value + 2.5f;
                } else if (value < 0.875f) {
                    r = 1.0f;
                    g = -4.0f * value + 3.5f;
                    b = 0.0f;
                } else {
                    r = -4.0f * value + 4.5f;
                    g = 0.0f;
                    b = 0.0f;
                }
                return ImVec4(r, g, b, 1.0f);
            }
            
            case ColorMap::HOT: {
                float r = std::min(1.0f, 3.0f * value);
                float g = std::min(1.0f, std::max(0.0f, 3.0f * value - 1.0f));
                float b = std::min(1.0f, std::max(0.0f, 3.0f * value - 2.0f));
                return ImVec4(r, g, b, 1.0f);
            }
            
            case ColorMap::COOL: {
                float r = value;
                float g = 1.0f - value;
                float b = 1.0f;
                return ImVec4(r, g, b, 1.0f);
            }
            
            case ColorMap::VIRIDIS: {
                // Viridis颜色映射的简化版本
                float r = 0.267004f + value * (0.993248f - 0.267004f) * 0.5f;
                float g = 0.004874f + value * (0.993248f - 0.004874f);
                float b = 0.329415f + value * (0.633929f - 0.329415f) * 2.0f;
                r = std::max(0.0f, std::min(1.0f, r));
                g = std::max(0.0f, std::min(1.0f, g));
                b = std::max(0.0f, std::min(1.0f, b));
                return ImVec4(r, g, b, 1.0f);
            }
            
            default:
                return ImVec4(value, value, value, 1.0f);
        }
    }

    void drawColorBar(ColorMap colorMap, float minVal, float maxVal, const ImVec2& size) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();
        
        // 绘制颜色条背景
        drawList->AddRectFilled(canvasPos, 
                               ImVec2(canvasPos.x + size.x, canvasPos.y + size.y),
                               IM_COL32(50, 50, 50, 255));
        
        // 绘制颜色渐变
        int numSteps = std::min(256, (int)size.y);
        float stepHeight = size.y / numSteps;
        
        for (int i = 0; i < numSteps; ++i) {
            float t = 1.0f - static_cast<float>(i) / (numSteps - 1); // 从上到下：高值到低值
            ImVec4 color = getColorFromValue(t, colorMap);
            ImU32 col32 = ImGui::ColorConvertFloat4ToU32(color);
            
            ImVec2 rectMin(canvasPos.x + 5, canvasPos.y + i * stepHeight);
            ImVec2 rectMax(canvasPos.x + size.x - 20, canvasPos.y + (i + 1) * stepHeight);
            
            drawList->AddRectFilled(rectMin, rectMax, col32);
        }
        
        // 绘制刻度线和标签
        int numTicks = 5;
        for (int i = 0; i <= numTicks; ++i) {
            float t = static_cast<float>(i) / numTicks;
            float y = canvasPos.y + t * size.y;
            float value = maxVal - t * (maxVal - minVal); // 从上到下：高值到低值
            
            // 刻度线
            drawList->AddLine(ImVec2(canvasPos.x + size.x - 20, y),
                             ImVec2(canvasPos.x + size.x - 15, y),
                             IM_COL32(255, 255, 255, 255), 1.0f);
            
            // 标签
            std::string label = formatNumber(value, 1);
            ImVec2 textSize = ImGui::CalcTextSize(label.c_str());
            drawList->AddText(ImVec2(canvasPos.x + size.x - 12, y - textSize.y * 0.5f),
                             IM_COL32(255, 255, 255, 255), label.c_str());
        }
        
        // 移动光标
        ImGui::SetCursorScreenPos(ImVec2(canvasPos.x, canvasPos.y + size.y + 5));
    }

    std::string formatNumber(double value, int precision) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision) << value;
        return oss.str();
    }

    void helpMarker(const char* desc) {
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
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
    autoApplyFilter(false),
    show3DWindow(false),
    framebufferID(0),
    colorTextureID(0), 
    depthTextureID(0),
    renderWidth(512),
    renderHeight(512){
        
    // 初始化统计数据
    originalStats = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    filteredStats = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
}

// 初始化NFD（在GUI初始化时调用）
bool initializeFileDialogs() {
#ifdef HAS_NATIVEFILEDIALOG
    // 旧版本 nativefiledialog 不需要显式初始化
    std::cout << "✓ nativefiledialog 就绪（旧版本API）" << std::endl;
    return true;
#else
    std::cout << "⚠ 使用终端文件输入模式" << std::endl;
    return false;
#endif
}

GUI::~GUI() {
    cleanup();
    cleanup3DRenderer();
}

bool GUI::loadChineseFont() {
    ImGuiIO& io = ImGui::GetIO();
    
    // 1. 首先加载默认英文字体作为基础
    ImFont* default_font = io.Fonts->AddFontDefault();
    
    // 2. 字体路径列表
    std::vector<std::string> font_paths = {
        "assets/fonts/NotoSansCJK-Regular.ttc",
        "assets/fonts/SourceHanSansCN-Regular.otf", 
        "assets/fonts/DroidSansFallbackFull.ttf",
        "assets/fonts/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf",
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/System/Library/Fonts/PingFang.ttc",
        "/System/Library/Fonts/Hiragino Sans GB.ttc"
    };
    
    // 3. 只加载中文字符范围
    static const ImWchar chinese_only_ranges[] = {
        0x2000, 0x206F, // General Punctuation
        0x3000, 0x30FF, // CJK Symbols and Punctuation, Hiragana, Katakana
        0x31F0, 0x31FF, // Katakana Phonetic Extensions
        0xFF00, 0xFFEF, // Half-width characters
        0x4e00, 0x9FAF, // CJK Ideograms
        0,
    };
    
    // 4. 尝试加载中文字体作为补充
    for (const auto& font_path : font_paths) {
        if (std::filesystem::exists(font_path)) {
            std::cout << "正在加载中文字体: " << font_path << std::endl;
            
            // 使用合并模式添加中文字体
            ImFontConfig config;
            config.MergeMode = true;           // 关键：合并模式
            config.PixelSnapH = true;
            config.GlyphMinAdvanceX = 13.0f;
            
            ImFont* chinese_font = io.Fonts->AddFontFromFileTTF(
                font_path.c_str(),
                18.0f,
                &config,                       // 传入配置
                chinese_only_ranges            // 只加载中文字符
            );
            
            if (chinese_font) {
                std::cout << "✓ 中文字体加载成功: " << font_path << std::endl;
                
                // 构建字体图集
                io.Fonts->Build();
                
                TextManager::setChineseSupport(true);
                return true;
            }
        }
    }
    
    // 5. 如果没有找到中文字体，确保至少有默认字体
    std::cout << "未找到中文字体，使用英文界面" << std::endl;
    if (!default_font) {
        io.Fonts->AddFontDefault();
    }
    io.Fonts->Build();
    
    TextManager::setChineseSupport(false);
    return false;
}

bool GUI::initialize() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    window = glfwCreateWindow(1400, 900, "FFT Image Processor - GUI", nullptr, nullptr);
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
    
    // 初始化文件对话框
    initializeFileDialogs();
    
    setupCallbacks();
    initializeGUI();
    
    // 创建默认测试图像
    processor->createTestImage(256);
    processor->fft2D();
    updateImageTextures();
    
    std::cout << "GUI initialized successfully!" << std::endl;
    
    return true;
}

void GUI::initializeGUI() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // 加载中文字体
    // io.Fonts->AddFontFromFileTTF("third_party/imgui/misc/fonts/SweiSansCJKjp-Regular.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
    
    // // 如果没有中文字体文件，使用默认字体并添加中文字形范围
    // if (io.Fonts->Fonts.empty()) {
    //     io.Fonts->AddFontDefault();
    //     io.Fonts->Build();
    // }
    loadChineseFont();
    
    ImGui::StyleColorsDark();
    
    // 调整样式使其更现代化
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.FrameRounding = 3.0f;
    style.ScrollbarRounding = 3.0f;
    style.GrabRounding = 3.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    
    // 调整颜色使其更加现代
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.9f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.2f, 0.5f);
    colors[ImGuiCol_Button] = ImVec4(0.3f, 0.3f, 0.8f, 0.8f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.4f, 0.4f, 0.9f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.2f, 0.2f, 0.7f, 1.0f);
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // 初始化3D渲染器
    init3DRenderer();
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
        if (show3DWindow) draw3DVisualizationWindow();
        
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
            if (ImGui::MenuItem("打开图像", "Ctrl+O")) {
                // 直接同步调用，不使用异步线程
                openImageFile();
            }
            if (ImGui::MenuItem("创建测试图像", "Ctrl+T")) {
                createTestImage();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("保存图像", "Ctrl+S", false, processor->getWidth() > 0)) {
                // 直接同步调用
                saveCurrentImage();
            }
            if (ImGui::MenuItem("另存为...", "Ctrl+Shift+S", false, processor->getWidth() > 0)) {
                // 直接同步调用
                saveImageAs();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("退出", "Alt+F4")) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("视图")) {
            ImGui::MenuItem("图像窗口", nullptr, &showImageWindow);
            ImGui::MenuItem("频域窗口", nullptr, &showFrequencyWindow);
            ImGui::MenuItem("控制面板", nullptr, &showControlWindow);
            ImGui::MenuItem("滤波器", nullptr, &showFilterWindow);
            ImGui::MenuItem("统计信息", nullptr, &showStatsWindow);
            ImGui::MenuItem("3D可视化", nullptr, &show3DWindow); 
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("工具")) {
            if (ImGui::MenuItem("重置滤波器")) {
                resetFilter();
            }
            if (ImGui::MenuItem("执行FFT", nullptr, false, processor->getWidth() > 0)) {
                processor->fft2D();
                updateImageTextures();
            }
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
    ImGui::Begin("图像显示", &showImageWindow, ImGuiWindowFlags_MenuBar);
    
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("显示模式")) {
            if (ImGui::MenuItem("原始图像", nullptr, currentDisplayMode == DisplayMode::ORIGINAL_IMAGE)) {
                currentDisplayMode = DisplayMode::ORIGINAL_IMAGE;
            }
            if (ImGui::MenuItem("频域幅度", nullptr, currentDisplayMode == DisplayMode::FREQUENCY_MAGNITUDE)) {
                currentDisplayMode = DisplayMode::FREQUENCY_MAGNITUDE;
            }
            if (ImGui::MenuItem("频域相位", nullptr, currentDisplayMode == DisplayMode::FREQUENCY_PHASE)) {
                currentDisplayMode = DisplayMode::FREQUENCY_PHASE;
            }
            if (ImGui::MenuItem("滤波后图像", nullptr, currentDisplayMode == DisplayMode::FILTERED_IMAGE)) {
                currentDisplayMode = DisplayMode::FILTERED_IMAGE;
            }
            if (ImGui::MenuItem("对比视图", nullptr, currentDisplayMode == DisplayMode::COMPARISON_VIEW)) {
                currentDisplayMode = DisplayMode::COMPARISON_VIEW;
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("颜色映射")) {
            if (ImGui::MenuItem("灰度", nullptr, currentColorMap == ColorMap::GRAYSCALE)) {
                currentColorMap = ColorMap::GRAYSCALE;
                updateImageTextures();
            }
            if (ImGui::MenuItem("Jet", nullptr, currentColorMap == ColorMap::JET)) {
                currentColorMap = ColorMap::JET;
                updateImageTextures();
            }
            if (ImGui::MenuItem("热度图", nullptr, currentColorMap == ColorMap::HOT)) {
                currentColorMap = ColorMap::HOT;
                updateImageTextures();
            }
            if (ImGui::MenuItem("冷色图", nullptr, currentColorMap == ColorMap::COOL)) {
                currentColorMap = ColorMap::COOL;
                updateImageTextures();
            }
            if (ImGui::MenuItem("Viridis", nullptr, currentColorMap == ColorMap::VIRIDIS)) {
                currentColorMap = ColorMap::VIRIDIS;
                updateImageTextures();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    
    if (processor->getWidth() > 0) {
        ImVec2 contentRegion = ImGui::GetContentRegionAvail();
        
        switch (currentDisplayMode) {
            case DisplayMode::ORIGINAL_IMAGE:
                drawImageWithLegend(originalImageTexture, processor->getWidth(), 
                                  processor->getHeight(), "原始图像", originalStats);
                break;
                
            case DisplayMode::FREQUENCY_MAGNITUDE:
                drawImageWithLegend(frequencyMagnitudeTexture, processor->getWidth(), 
                                  processor->getHeight(), "频域幅度", originalStats);
                break;
                
            case DisplayMode::FREQUENCY_PHASE:
                drawImageWithLegend(frequencyPhaseTexture, processor->getWidth(), 
                                  processor->getHeight(), "频域相位", originalStats);
                break;
                
            case DisplayMode::FILTERED_IMAGE:
                drawImageWithLegend(filteredImageTexture, processor->getWidth(), 
                                  processor->getHeight(), "滤波后图像", filteredStats);
                break;
                
            case DisplayMode::COMPARISON_VIEW:
                // 绘制对比视图
                ImVec2 imageSize(contentRegion.x / 2 - 10, contentRegion.y);
                
                ImGui::BeginChild("Original", ImVec2(imageSize.x, 0), true);
                drawImageWithLegend(originalImageTexture, processor->getWidth(), 
                                  processor->getHeight(), "原始图像", originalStats);
                ImGui::EndChild();
                
                ImGui::SameLine();
                
                ImGui::BeginChild("Filtered", ImVec2(imageSize.x, 0), true);
                drawImageWithLegend(filteredImageTexture, processor->getWidth(), 
                                  processor->getHeight(), "滤波后图像", filteredStats);
                ImGui::EndChild();
                break;
        }
    } else {
        ImGui::Text("没有加载图像");
        ImGui::Text("请从文件菜单加载图像或创建测试图像");
        ImGui::Spacing();
        if (ImGui::Button("创建测试图像")) {
            createTestImage();
        }
    }
    
    ImGui::End();
}

void GUI::drawImageWithLegend(GLuint texture, int width, int height, 
                            const std::string& title, const ImageStats& stats) {
    ImVec2 contentRegion = ImGui::GetContentRegionAvail();
    
    // 计算图像显示尺寸（保持宽高比）
    float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    ImVec2 imageSize;
    
    if (contentRegion.x / aspectRatio <= contentRegion.y - 120) {
        imageSize.x = contentRegion.x - 80;
        imageSize.y = imageSize.x / aspectRatio;
    } else {
        imageSize.y = contentRegion.y - 120;
        imageSize.x = imageSize.y * aspectRatio;
    }
    
    // 确保最小尺寸
    imageSize.x = std::max(imageSize.x, 100.0f);
    imageSize.y = std::max(imageSize.y, 100.0f);
    
    // 绘制标题
    ImGui::Text("%s (%dx%d)", title.c_str(), width, height);
    ImGui::Separator();
    
    if (texture != 0) {
        // 绘制图像
        ImGui::Image(reinterpret_cast<void*>(texture), imageSize);
        
        // 在图像旁边绘制颜色条
        ImGui::SameLine();
        ImVec2 colorBarSize(60, imageSize.y);
        GuiUtils::drawColorBar(currentColorMap, stats.minValue, stats.maxValue, colorBarSize);
    } else {
        // 如果纹理无效，显示占位符
        ImGui::Button("图像加载中...", imageSize);
    }
    
    // 绘制图像统计信息
    ImGui::Separator();
    ImGui::Text("统计信息:");
    ImGui::Columns(2, "stats", false);
    
    ImGui::Text("最小值: %s", GuiUtils::formatNumber(stats.minValue, 2).c_str());
    ImGui::NextColumn();
    ImGui::Text("最大值: %s", GuiUtils::formatNumber(stats.maxValue, 2).c_str());
    ImGui::NextColumn();
    
    ImGui::Text("平均值: %s", GuiUtils::formatNumber(stats.meanValue, 2).c_str());
    ImGui::NextColumn();
    ImGui::Text("标准差: %s", GuiUtils::formatNumber(stats.stdValue, 2).c_str());
    ImGui::NextColumn();
    
    if (currentDisplayMode == DisplayMode::FILTERED_IMAGE && filteredStats.mse > 0) {
        ImGui::Text("MSE: %s", GuiUtils::formatNumber(filteredStats.mse, 4).c_str());
        ImGui::NextColumn();
        ImGui::Text("PSNR: %s dB", GuiUtils::formatNumber(filteredStats.psnr, 2).c_str());
        ImGui::NextColumn();
        ImGui::Text("SSIM: %s", GuiUtils::formatNumber(filteredStats.ssim, 4).c_str());
        ImGui::NextColumn();
        ImGui::Text(""); // 空列
    }
    
    ImGui::Columns(1);
}

void GUI::drawFrequencyWindow() {
    ImGui::Begin("频域分析", &showFrequencyWindow);
    
    if (processor->getWidth() > 0) {
        ImGui::Text("频域分析 - 图像尺寸: %dx%d", processor->getWidth(), processor->getHeight());
        ImGui::Separator();
        
        drawFrequencyPlot();
        
        ImGui::Separator();
        
        if (ImGui::BeginTabBar("FrequencyTabs")) {
            if (ImGui::BeginTabItem("幅度分布")) {
                drawHistogram(processor->getGrayImage(), "像素值分布");
                ImGui::EndTabItem();
            }
            
            if (ImGui::BeginTabItem("频域信息")) {
                ImGui::Text("FFT变换已完成");
                ImGui::Text("频域数据可用于滤波处理");
                
                if (ImGui::Button("重新计算FFT")) {
                    processor->fft2D();
                    updateImageTextures();
                }
                ImGui::EndTabItem();
            }
            
            ImGui::EndTabBar();
        }
    } else {
        ImGui::Text("请先加载图像以查看频域分析");
        if (ImGui::Button("创建测试图像")) {
            createTestImage();
        }
    }
    
    ImGui::End();
}

void GUI::drawControlWindow() {
    ImGui::Begin("控制面板", &showControlWindow);
    
    ImGui::Text("图像信息:");
    ImGui::Separator();
    
    if (processor->getWidth() > 0) {
        ImGui::Text("尺寸: %dx%d", processor->getWidth(), processor->getHeight());
        ImGui::Text("通道数: %d", processor->getOriginalChannels());
        ImGui::Text("文件: %s", currentImagePath.empty() ? "测试图像" : currentImagePath.c_str());
        
        ImGui::Spacing();
        ImGui::Text("像素总数: %d", processor->getWidth() * processor->getHeight());
        
        if (processor->getGrayImage().size() > 0) {
            ImGui::Text("数据状态: 已加载");
        }
    } else {
        ImGui::Text("未加载图像");
    }
    
    ImGui::Separator();
    ImGui::Text("显示控制:");
    
    if (ImGui::Button("重置视图", ImVec2(-1, 0))) {
        currentDisplayMode = DisplayMode::ORIGINAL_IMAGE;
        currentColorMap = ColorMap::GRAYSCALE;
        updateImageTextures();
    }
    
    ImGui::Separator();
    ImGui::Text("快速操作:");
    
    if (ImGui::Button("执行FFT", ImVec2(-1, 0))) {
        if (processor->getWidth() > 0) {
            processor->fft2D();
            updateImageTextures();
            ImGui::OpenPopup("FFT完成");
        }
    }
    
    if (ImGui::Button("应用滤波器", ImVec2(-1, 0))) {
        applyCurrentFilter();
    }
    
    if (ImGui::Button("重置滤波器", ImVec2(-1, 0))) {
        resetFilter();
    }
    
    // 弹出提示
    if (ImGui::BeginPopupModal("FFT完成", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("FFT变换已完成！");
        ImGui::Separator();
        if (ImGui::Button("确定", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    
    ImGui::End();
}

void GUI::drawFilterWindow() {
    ImGui::Begin("滤波器设置", &showFilterWindow);
    
    ImGui::Text("滤波器类型:");
    bool typeChanged = false;
    
    if (ImGui::RadioButton("低通滤波", &filterType, 0)) typeChanged = true;
    ImGui::SameLine();
    GuiUtils::helpMarker("保留低频成分，去除高频噪声");
    
    if (ImGui::RadioButton("高通滤波", &filterType, 1)) typeChanged = true;
    ImGui::SameLine();
    GuiUtils::helpMarker("保留高频成分，增强边缘细节");
    
    if (ImGui::RadioButton("带通滤波", &filterType, 2)) typeChanged = true;
    ImGui::SameLine();
    GuiUtils::helpMarker("只保留特定频率范围的成分");
    
    ImGui::Separator();
    
    bool paramChanged = false;
    
    switch (filterType) {
        case 0: // 低通
            if (ImGui::SliderFloat("截止频率", &lowPassCutoff, 0.01f, 1.0f, "%.3f")) {
                paramChanged = true;
            }
            ImGui::Text("当前设置: 保留 %.1f%% 的低频成分", lowPassCutoff * 100);
            break;
            
        case 1: // 高通  
            if (ImGui::SliderFloat("截止频率", &highPassCutoff, 0.01f, 1.0f, "%.3f")) {
                paramChanged = true;
            }
            ImGui::Text("当前设置: 移除 %.1f%% 的低频成分", highPassCutoff * 100);
            break;
            
        case 2: // 带通
            if (ImGui::SliderFloat("低截止频率", &bandPassLow, 0.01f, 0.99f, "%.3f")) {
                paramChanged = true;
            }
            if (ImGui::SliderFloat("高截止频率", &bandPassHigh, 0.02f, 1.0f, "%.3f")) {
                paramChanged = true;
            }
            
            if (bandPassLow >= bandPassHigh) {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "警告: 低截止频率应小于高截止频率");
                if (bandPassLow >= bandPassHigh) {
                    bandPassHigh = bandPassLow + 0.01f;
                }
            } else {
                ImGui::Text("频率范围: %.1f%% - %.1f%%", bandPassLow * 100, bandPassHigh * 100);
            }
            break;
    }
    
    if (typeChanged || paramChanged) {
        onFilterParameterChanged();
    }
    
    ImGui::Separator();
    
    if (ImGui::Checkbox("自动应用", &autoApplyFilter)) {
        if (autoApplyFilter) {
            applyCurrentFilter();
        }
    }
    ImGui::SameLine();
    GuiUtils::helpMarker("启用后，参数改变时自动应用滤波器");
    
    if (ImGui::Button("手动应用", ImVec2(-1, 0))) {
        applyCurrentFilter();
    }
    
    if (ImGui::Button("重置参数", ImVec2(-1, 0))) {
        resetFilter();
    }
    
    ImGui::Separator();
    ImGui::Text("预览效果:");
    ImGui::Text("原图 → 滤波器 → 结果图");
    ImGui::Text("可在图像窗口中切换查看效果");
    
    ImGui::End();
}

void GUI::drawStatsWindow() {
    ImGui::Begin("统计信息", &showStatsWindow);
    
    if (processor->getWidth() > 0) {
        ImGui::Text("原始图像统计:");
        ImGui::Separator();
        
        ImGui::Columns(2, "original_stats", false);
        ImGui::Text("图像尺寸");
        ImGui::NextColumn();
        ImGui::Text("%dx%d", processor->getWidth(), processor->getHeight());
        ImGui::NextColumn();
        
        ImGui::Text("最小值");
        ImGui::NextColumn();
        ImGui::Text("%.3f", originalStats.minValue);
        ImGui::NextColumn();
        
        ImGui::Text("最大值");
        ImGui::NextColumn();
        ImGui::Text("%.3f", originalStats.maxValue);
        ImGui::NextColumn();
        
        ImGui::Text("平均值");
        ImGui::NextColumn();
        ImGui::Text("%.3f", originalStats.meanValue);
        ImGui::NextColumn();
        
        ImGui::Text("标准差");
        ImGui::NextColumn();
        ImGui::Text("%.3f", originalStats.stdValue);
        ImGui::NextColumn();
        
        ImGui::Columns(1);
        
        ImGui::Spacing();
        
        // 原始图像直方图
        ImGui::Text("像素值分布:");
        drawHistogram(processor->getGrayImage(), "原始图像直方图");
        
        ImGui::Spacing();
        
        if (filteredImageTexture && filteredStats.mse >= 0) {
            ImGui::Text("滤波后图像统计:");
            ImGui::Separator();
            
            ImGui::Columns(2, "filtered_stats", false);
            
            ImGui::Text("MSE (均方误差)");
            ImGui::NextColumn();
            ImGui::Text("%.6f", filteredStats.mse);
            ImGui::NextColumn();
            
            ImGui::Text("PSNR (峰值信噪比)");
            ImGui::NextColumn();
            ImGui::Text("%.2f dB", filteredStats.psnr);
            ImGui::NextColumn();
            
            ImGui::Text("SSIM (结构相似性)");
            ImGui::NextColumn();
            ImGui::Text("%.4f", filteredStats.ssim);
            ImGui::NextColumn();
            
            ImGui::Text("最小值");
            ImGui::NextColumn();
            ImGui::Text("%.3f", filteredStats.minValue);
            ImGui::NextColumn();
            
            ImGui::Text("最大值");
            ImGui::NextColumn();
            ImGui::Text("%.3f", filteredStats.maxValue);
            ImGui::NextColumn();
            
            ImGui::Text("平均值");
            ImGui::NextColumn();
            ImGui::Text("%.3f", filteredStats.meanValue);
            ImGui::NextColumn();
            
            ImGui::Text("标准差");
            ImGui::NextColumn();
            ImGui::Text("%.3f", filteredStats.stdValue);
            ImGui::NextColumn();
            
            ImGui::Columns(1);
            
            // 质量评估指标解释
            ImGui::Spacing();
            ImGui::Text("质量指标说明:");
            ImGui::BulletText("MSE: 均方误差，越小越好 (0表示完全相同)");
            ImGui::BulletText("PSNR: 峰值信噪比，越大越好 (通常>30dB为良好)");
            ImGui::BulletText("SSIM: 结构相似性，越接近1越好 (1表示完全相同)");
        }
    } else {
        ImGui::Text("请先加载图像");
        if (ImGui::Button("创建测试图像")) {
            createTestImage();
        }
    }
    
    ImGui::End();
}

void GUI::drawAboutWindow() {
    ImGui::Begin("关于", &showAboutWindow, ImGuiWindowFlags_AlwaysAutoResize);
    
    ImGui::Text("FFT 图像处理器 - GUI版本");
    ImGui::Text("版本: 2.0.0");
    ImGui::Separator();
    
    ImGui::Text("功能特性:");
    ImGui::BulletText("2D 快速傅里叶变换 (FFT)");
    ImGui::BulletText("多种数字滤波器 (低通/高通/带通)");
    ImGui::BulletText("实时参数调整和预览");
    ImGui::BulletText("图像质量评估 (MSE/PSNR/SSIM)");
    ImGui::BulletText("多种颜色映射显示");
    ImGui::BulletText("完整的统计分析");
    
    ImGui::Spacing();
    ImGui::Text("技术栈:");
    ImGui::BulletText("ImGui - 现代化用户界面");
    ImGui::BulletText("ImPlot - 专业数据可视化");
    ImGui::BulletText("OpenGL - 高性能图形渲染");
    ImGui::BulletText("GLFW - 跨平台窗口管理");
    ImGui::BulletText("STB - 轻量级图像处理");
    
    ImGui::Spacing();
    ImGui::Text("快捷键:");
    ImGui::BulletText("Ctrl+O: 打开图像文件");
    ImGui::BulletText("Ctrl+S: 保存当前图像");
    ImGui::BulletText("Ctrl+T: 创建测试图像");
    ImGui::BulletText("拖拽: 直接拖拽图像文件到窗口");
    
    ImGui::Spacing();
    ImGui::Text("使用提示:");
    ImGui::BulletText("支持 PNG, JPG, BMP 等常见格式");
    ImGui::BulletText("图像会自动调整为2的幂次尺寸");
    ImGui::BulletText("可实时调整滤波器参数查看效果");
    ImGui::BulletText("所有窗口都可独立移动和调整");
    
    ImGui::Spacing();
    if (ImGui::Button("关闭", ImVec2(120, 0))) {
        showAboutWindow = false;
    }
    
    ImGui::End();
}

void GUI::drawFrequencyPlot() {
    if (processor->getWidth() == 0) return;
    
    // 创建频谱数据用于绘制
    const auto& freqDomain = processor->getFrequencyDomain();
    int width = processor->getWidth();
    int height = processor->getHeight();
    
    if (freqDomain.empty() || freqDomain.size() != height) {
        ImGui::Text("频域数据不可用，请先执行FFT");
        return;
    }
    
    // 计算中心线的频谱（水平和垂直）
    std::vector<float> freqX, freqY;
    std::vector<float> magnitudeX, magnitudeY;
    
    // 水平中心线频谱
    int centerY = height / 2;
    for (int x = 0; x < width; ++x) {
        freqX.push_back(static_cast<float>(x - width/2));
        double mag = freqDomain[centerY][x].magnitude();
        magnitudeX.push_back(static_cast<float>(std::log(1.0 + mag)));
    }
    
    // 垂直中心线频谱
    int centerX = width / 2;
    for (int y = 0; y < height; ++y) {
        freqY.push_back(static_cast<float>(y - height/2));
        double mag = freqDomain[y][centerX].magnitude();
        magnitudeY.push_back(static_cast<float>(std::log(1.0 + mag)));
    }
    
    // 绘制频谱图
    if (ImPlot::BeginPlot("频域分析 (中心切片)", ImVec2(-1, 300))) {
        ImPlot::SetupAxes("频率位置", "幅度 (对数)");
        
        if (!magnitudeX.empty()) {
            ImPlot::PlotLine("水平频谱", freqX.data(), magnitudeX.data(), width);
        }
        if (!magnitudeY.empty()) {
            ImPlot::PlotLine("垂直频谱", freqY.data(), magnitudeY.data(), height);
        }
        
        ImPlot::EndPlot();
    }
}

void GUI::drawHistogram(const std::vector<std::vector<double>>& image, const std::string& title) {
    if (image.empty()) return;
    
    // 计算直方图
    const int numBins = 64; // 减少bin数量提高性能
    std::vector<float> bins(numBins, 0.0f);
    std::vector<float> binCenters(numBins);
    
    // 找到数据范围
    double minVal = image[0][0];
    double maxVal = image[0][0];
    for (const auto& row : image) {
        for (double val : row) {
            minVal = std::min(minVal, val);
            maxVal = std::max(maxVal, val);
        }
    }
    
    if (maxVal == minVal) {
        ImGui::Text("图像数据无变化");
        return;
    }
    
    // 计算bin范围
    double binWidth = (maxVal - minVal) / numBins;
    for (int i = 0; i < numBins; ++i) {
        binCenters[i] = minVal + (i + 0.5) * binWidth;
    }
    
    // 统计像素值
    for (const auto& row : image) {
        for (double val : row) {
            int binIndex = static_cast<int>((val - minVal) / binWidth);
            binIndex = std::max(0, std::min(numBins - 1, binIndex));
            bins[binIndex] += 1.0f;
        }
    }
    
    // 绘制直方图
    if (ImPlot::BeginPlot(title.c_str(), ImVec2(-1, 200))) {
        ImPlot::SetupAxes("像素值", "频数");
        ImPlot::PlotBars("分布", binCenters.data(), bins.data(), numBins, binWidth * 0.8);
        ImPlot::EndPlot();
    }
}

// 实现纹理更新方法
void GUI::updateImageTextures() {
    if (processor->getWidth() == 0) return;
    
    // 计算原始图像统计
    calculateImageStats(processor->getGrayImage(), originalStats);
    
    // 创建或更新原始图像纹理
    originalImageTexture = createTextureFromImage(processor->getGrayImage(), 
                                                 processor->getWidth(), 
                                                 processor->getHeight(), 
                                                 ColorMap::GRAYSCALE);
    
    // 创建频域幅度和相位图像
    const auto& freqDomain = processor->getFrequencyDomain();
    if (!freqDomain.empty()) {
        std::vector<std::vector<double>> magnitudeImage(processor->getHeight(), 
                                                        std::vector<double>(processor->getWidth()));
        std::vector<std::vector<double>> phaseImage(processor->getHeight(), 
                                                   std::vector<double>(processor->getWidth()));
        
        for (int y = 0; y < processor->getHeight(); ++y) {
            for (int x = 0; x < processor->getWidth(); ++x) {
                magnitudeImage[y][x] = std::log(1.0 + freqDomain[y][x].magnitude());
                phaseImage[y][x] = freqDomain[y][x].phase() + M_PI; // 偏移到0-2π范围
            }
        }
        
        frequencyMagnitudeTexture = createTextureFromImage(magnitudeImage, 
                                                          processor->getWidth(), 
                                                          processor->getHeight(), 
                                                          currentColorMap);
        
        frequencyPhaseTexture = createTextureFromImage(phaseImage, 
                                                      processor->getWidth(), 
                                                      processor->getHeight(), 
                                                      currentColorMap);
    }
    
    // 应用当前滤波器
    applyCurrentFilter();
}

GLuint GUI::createTextureFromImage(const std::vector<std::vector<double>>& image, 
                                   int width, int height, ColorMap colorMap) {
    if (image.empty() || width <= 0 || height <= 0) {
        return 0;
    }
    
    // 找到最大值和最小值用于归一化
    double minVal = image[0][0];
    double maxVal = image[0][0];
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (y < image.size() && x < image[y].size()) {
                minVal = std::min(minVal, image[y][x]);
                maxVal = std::max(maxVal, image[y][x]);
            }
        }
    }
    
    // 创建RGBA颜色数据
    std::vector<unsigned char> colorData(width * height * 4);
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double value = 0.0;
            if (y < image.size() && x < image[y].size()) {
                value = image[y][x];
            }
            
            // 归一化值
            double normalized = (maxVal != minVal) ? (value - minVal) / (maxVal - minVal) : 0.0;
            ImVec4 color = GuiUtils::getColorFromValue(normalized, colorMap);
            
            int idx = (y * width + x) * 4;
            colorData[idx + 0] = static_cast<unsigned char>(color.x * 255);
            colorData[idx + 1] = static_cast<unsigned char>(color.y * 255);
            colorData[idx + 2] = static_cast<unsigned char>(color.z * 255);
            colorData[idx + 3] = 255; // Alpha
        }
    }
    
    // 创建OpenGL纹理
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, colorData.data());
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return textureID;
}

void GUI::calculateImageStats(const std::vector<std::vector<double>>& image, ImageStats& stats) {
    if (image.empty() || image[0].empty()) {
        stats = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        return;
    }
    
    int height = image.size();
    int width = image[0].size();
    
    stats.minValue = image[0][0];
    stats.maxValue = image[0][0];
    stats.meanValue = 0.0;
    
    // 计算最小值、最大值和平均值
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double val = image[y][x];
            stats.minValue = std::min(stats.minValue, val);
            stats.maxValue = std::max(stats.maxValue, val);
            stats.meanValue += val;
        }
    }
    stats.meanValue /= (width * height);
    
    // 计算标准差
    stats.stdValue = 0.0;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double diff = image[y][x] - stats.meanValue;
            stats.stdValue += diff * diff;
        }
    }
    stats.stdValue = std::sqrt(stats.stdValue / (width * height));
}

void GUI::applyCurrentFilter() {
    if (processor->getWidth() == 0) return;
    
    std::vector<std::vector<Complex>> filteredFreq;
    
    switch (filterType) {
        case 0: // 低通
            filteredFreq = processor->lowPassFilter(lowPassCutoff);
            break;
        case 1: // 高通
            filteredFreq = processor->highPassFilter(highPassCutoff);
            break;
        case 2: // 带通
            if (bandPassLow < bandPassHigh) {
                filteredFreq = processor->bandPassFilter(bandPassLow, bandPassHigh);
            } else {
                filteredFreq = processor->getFrequencyDomain();
            }
            break;
        default:
            filteredFreq = processor->getFrequencyDomain();
            break;
    }
    
    // 重建图像
    auto filteredImage = processor->ifft2D(filteredFreq);
    
    // 创建滤波后图像纹理
    if (filteredImageTexture) {
        glDeleteTextures(1, &filteredImageTexture);
    }
    filteredImageTexture = createTextureFromImage(filteredImage, 
                                                 processor->getWidth(), 
                                                 processor->getHeight(), 
                                                 ColorMap::GRAYSCALE);
    
    // 计算滤波后图像统计信息
    calculateImageStats(filteredImage, filteredStats);
    
    // 计算质量指标
    if (!processor->getGrayImage().empty()) {
        filteredStats.mse = processor->calculateMSE(processor->getGrayImage(), filteredImage);
        filteredStats.psnr = processor->calculatePSNR(filteredStats.mse);
        filteredStats.ssim = processor->calculateSSIM(processor->getGrayImage(), filteredImage);
    }
}

void GUI::cleanup() {
    if (originalImageTexture) glDeleteTextures(1, &originalImageTexture);
    if (frequencyMagnitudeTexture) glDeleteTextures(1, &frequencyMagnitudeTexture);
    if (frequencyPhaseTexture) glDeleteTextures(1, &frequencyPhaseTexture);
    if (filteredImageTexture) glDeleteTextures(1, &filteredImageTexture);

    // 清理文件对话框
    cleanupFileDialogs();
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

// 清理函数
void GUI::cleanup3DRenderer() {
    if (framebufferID) {
        glDeleteFramebuffers(1, &framebufferID);
        glDeleteTextures(1, &colorTextureID);
        glDeleteTextures(1, &depthTextureID);
    }
    openglRenderer.reset();
}

void GUI::dropCallback(GLFWwindow* window, int count, const char** paths) {
    GUI* gui = static_cast<GUI*>(glfwGetWindowUserPointer(window));
    if (gui && count > 0) {
        gui->currentImagePath = paths[0];
        std::cout << "拖拽文件: " << paths[0] << std::endl;
        if (gui->processor->loadImage(paths[0])) {
            gui->processor->fft2D();
            gui->updateImageTextures();
            std::cout << "图像加载成功！" << std::endl;
        } else {
            std::cout << "图像加载失败！" << std::endl;
        }
    }
}

void GUI::keyCallback(GLFWwindow* window, int key, int, int action, int mods) {
    GUI* gui = static_cast<GUI*>(glfwGetWindowUserPointer(window));
    if (gui && action == GLFW_PRESS && (mods & GLFW_MOD_CONTROL)) {
        switch (key) {
            case GLFW_KEY_O:
                gui->openImageFile();  // 直接同步调用
                break;
            case GLFW_KEY_S:
                if (mods & GLFW_MOD_SHIFT) {
                    gui->saveImageAs();  // 直接同步调用
                } else {
                    gui->saveCurrentImage();  // 直接同步调用
                }
                break;
            case GLFW_KEY_T:
                gui->createTestImage();
                break;
        }
    }
}

bool GUI::openImageFile() {
    std::cout << "正在打开文件..." << std::endl;
    
    std::string filename = SimpleFileDialog::openFile("");
    
    if (!filename.empty()) {
        currentImagePath = filename;
        std::cout << "尝试加载: " << filename << std::endl;
        
        if (processor->loadImage(filename)) {
            processor->fft2D();
            updateImageTextures();
            std::cout << "✓ 图像加载成功!" << std::endl;
            return true;
        } else {
            std::cout << "✗ 图像加载失败，请检查文件格式" << std::endl;
        }
    }
    
    return false;
}

bool GUI::saveImageAs() {
    if (processor->getWidth() == 0) {
        std::cout << "✗ 没有图像可保存" << std::endl;
        return false;
    }
    
    std::string filename = SimpleFileDialog::saveFile("");
    
    if (!filename.empty()) {
        lastSavePath = filename;
        return saveCurrentImage();
    }
    
    return false;
}

bool GUI::saveCurrentImage() {
    if (processor->getWidth() == 0) {
        std::cout << "✗ 没有图像可保存" << std::endl;
        return false;
    }
    
    if (lastSavePath.empty()) {
        return saveImageAs();
    }
    
    try {
        // 根据当前显示模式保存相应的图像
        std::vector<std::vector<double>> imageToSave;
        
        switch (currentDisplayMode) {
            case DisplayMode::ORIGINAL_IMAGE:
                imageToSave = processor->getGrayImage();
                break;
            case DisplayMode::FILTERED_IMAGE:
                // 获取当前滤波后的图像
                {
                    std::vector<std::vector<Complex>> filteredFreq;
                    switch (filterType) {
                        case 0: filteredFreq = processor->lowPassFilter(lowPassCutoff); break;
                        case 1: filteredFreq = processor->highPassFilter(highPassCutoff); break;
                        case 2: filteredFreq = processor->bandPassFilter(bandPassLow, bandPassHigh); break;
                        default: filteredFreq = processor->getFrequencyDomain(); break;
                    }
                    imageToSave = processor->ifft2D(filteredFreq);
                }
                break;
            default:
                imageToSave = processor->getGrayImage();
                break;
        }
        
        bool success = processor->saveImage(lastSavePath, imageToSave);
        if (success) {
            std::cout << "✓ 图像保存成功: " << lastSavePath << std::endl;
        } else {
            std::cout << "✗ 图像保存失败: " << lastSavePath << std::endl;
        }
        return success;
    } catch (const std::exception& e) {
        std::cerr << "保存图像时发生错误: " << e.what() << std::endl;
        return false;
    }
}

void GUI::createTestImage() {
    processor->createTestImage(256);
    processor->fft2D();
    updateImageTextures();
    currentImagePath = "";
    std::cout << "测试图像创建完成" << std::endl;
}

void GUI::resetFilter() {
    filterType = 0;
    lowPassCutoff = 0.5f;
    highPassCutoff = 0.5f;
    bandPassLow = 0.2f;
    bandPassHigh = 0.8f;
    applyCurrentFilter();
}

void GUI::onFilterParameterChanged() {
    if (autoApplyFilter && processor->getWidth() > 0) {
        applyCurrentFilter();
    }
}

// 3D渲染器初始化
void GUI::init3DRenderer() {
    openglRenderer = std::make_unique<OpenGLRenderer>();
    
    // 不初始化窗口，只初始化OpenGL资源
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW for 3D renderer" << std::endl;
        return;
    }
    
    // 设置3D渲染的帧缓冲
    setup3DFramebuffer(renderWidth, renderHeight);
    
    // 手动初始化OpenGL渲染器的着色器和数据（不创建窗口）
    openglRenderer->initializeWithoutWindow();
}

// 设置离屏渲染的帧缓冲
void GUI::setup3DFramebuffer(int width, int height) {
    // 清理旧的帧缓冲
    if (framebufferID) {
        glDeleteFramebuffers(1, &framebufferID);
        glDeleteTextures(1, &colorTextureID);
        glDeleteTextures(1, &depthTextureID);
    }
    
    renderWidth = width;
    renderHeight = height;
    
    // 创建帧缓冲对象
    glGenFramebuffers(1, &framebufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
    
    // 创建颜色纹理
    glGenTextures(1, &colorTextureID);
    glBindTexture(GL_TEXTURE_2D, colorTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTextureID, 0);
    
    // 创建深度纹理
    glGenTextures(1, &depthTextureID);
    glBindTexture(GL_TEXTURE_2D, depthTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTextureID, 0);
    
    // 检查帧缓冲完整性
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer not complete!" << std::endl;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// 渲染3D场景到纹理
void GUI::render3DToTexture() {
    if (!openglRenderer || !framebufferID) return;
    
    // 绑定帧缓冲
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
    glViewport(0, 0, renderWidth, renderHeight);
    
    // 清除颜色和深度缓冲
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    
    // 设置3D频域数据
    if (processor->getWidth() > 0) {
        std::vector<float> vertices = processor->getFrequencyVisualizationData();
        openglRenderer->setupVertexData(vertices);
        
        // 渲染3D频域
        int pointCount = vertices.size() / 6; // 每个点6个float (xyz + rgb)
        openglRenderer->render3DFrequency(pointCount);
        openglRenderer->renderAxes();
    }
    
    // 恢复默认帧缓冲
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // 恢复ImGui的视口
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
}

// 绘制3D可视化窗口
void GUI::draw3DVisualizationWindow() {
    // 设置窗口标志，禁用移动（如果你不需要移动窗口）
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None;

    ImGui::Begin("3D频域可视化", &show3DWindow);
    
    if (processor->getWidth() > 0) {
        // 控制面板
        ImGui::Text("3D可视化控制");
        ImGui::Separator();
        
        // 分辨率控制
        static int resolution = 512;
        if (ImGui::SliderInt(T("渲染分辨率"), &resolution, 256, 1024)) {
            setup3DFramebuffer(resolution, resolution);
        }
        
        // 鼠标控制提示
        ImGui::Text("鼠标控制");
        ImGui::BulletText("左键拖拽: 旋转视角");
        ImGui::BulletText("滚轮: 缩放");
        ImGui::Separator();
        
        // 渲染3D场景到纹理
        render3DToTexture();
        
        // 在ImGui中显示渲染结果
        ImVec2 contentRegion = ImGui::GetContentRegionAvail();
        float aspectRatio = 1.0f; // 正方形
        ImVec2 imageSize;
        if (contentRegion.x <= contentRegion.y) {
            imageSize.x = contentRegion.x - 20;
            imageSize.y = imageSize.x;
        } else {
            imageSize.y = contentRegion.y - 20;
            imageSize.x = imageSize.y;
        }

        // 记录图像开始位置
        ImVec2 imageStartPos = ImGui::GetCursorScreenPos();
        
        // 显示3D渲染结果
        ImGui::Image(reinterpret_cast<void*>(colorTextureID), imageSize, ImVec2(0, 1), ImVec2(1, 0));

        // 检查鼠标是否在图像区域内
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 mousePos = io.MousePos;
        bool mouseInImage = (mousePos.x >= imageStartPos.x && 
                            mousePos.x <= imageStartPos.x + imageSize.x &&
                            mousePos.y >= imageStartPos.y && 
                            mousePos.y <= imageStartPos.y + imageSize.y);
        
        // 处理鼠标交互
        if (mouseInImage && openglRenderer) {
            static bool was3DInteracting = false;
            bool is3DInteracting = false;
            
            // 鼠标拖拽旋转
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                ImVec2 delta = io.MouseDelta;
                openglRenderer->handleMouseDrag(delta.x, delta.y);
                is3DInteracting = true;
            }
            
            // 鼠标滚轮缩放
            if (io.MouseWheel != 0) {
                openglRenderer->handleMouseWheel(io.MouseWheel);
                is3DInteracting = true;
            }
            
            // 当开始3D交互时，捕获鼠标
            if (is3DInteracting && !was3DInteracting) {
                ImGui::SetWindowFocus(); // 确保窗口有焦点
            }
            
            // 在3D交互期间阻止ImGui的默认行为
            if (is3DInteracting) {
                io.WantCaptureMouse = false;
            }
            
            was3DInteracting = is3DInteracting;
        }
        
        ImGui::Spacing();
        ImGui::Text("频域3D显示");
        ImGui::Text("点数: %d", processor->getWidth() * processor->getHeight());
        
    } else {
        ImGui::Text("请先加载图像以查看3D可视化");
        if (ImGui::Button(T("创建测试图像"))) {
            createTestImage();
        }
    }
    
    ImGui::End();
}