#include "include/GUI.h"
#include <iostream>
#include <filesystem>

bool GUI::loadChineseFont() {
    ImGuiIO& io = ImGui::GetIO();
    
    // 字体文件路径列表（按优先级排序）
    std::vector<std::string> font_paths = {
        "assets/fonts/NotoSansCJK-Regular.ttc",
        "assets/fonts/SourceHanSansCN-Regular.otf", 
        "assets/fonts/DroidSansFallbackFull.ttf",
        "assets/fonts/DejaVuSans.ttf",
        "third_party/imgui/misc/fonts/DroidSans.ttf",
        "/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf",
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
        "/System/Library/Fonts/PingFang.ttc",
        "/System/Library/Fonts/Hiragino Sans GB.ttc"
    };
    
    // 尝试加载字体
    for (const auto& font_path : font_paths) {
        if (std::filesystem::exists(font_path)) {
            std::cout << "尝试加载字体: " << font_path << std::endl;
            
            // 配置中文字符范围
            static const ImWchar chinese_ranges[] = {
                0x0020, 0x00FF, // Basic Latin + Latin Supplement
                0x2000, 0x206F, // General Punctuation
                0x3000, 0x30FF, // CJK Symbols and Punctuation, Hiragana, Katakana
                0x31F0, 0x31FF, // Katakana Phonetic Extensions
                0xFF00, 0xFFEF, // Half-width characters
                0x4e00, 0x9FAF, // CJK Ideograms
                0,
            };
            
            // 尝试不同的字体大小
            std::vector<float> font_sizes = {16.0f, 18.0f, 14.0f, 20.0f};
            
            for (float size : font_sizes) {
                ImFont* font = io.Fonts->AddFontFromFileTTF(
                    font_path.c_str(), 
                    size, 
                    nullptr, 
                    chinese_ranges
                );
                
                if (font) {
                    std::cout << "✓ 成功加载中文字体: " << font_path 
                              << " (大小: " << size << ")" << std::endl;
                    
                    // 构建字体图集
                    io.Fonts->Build();
                    return true;
                }
            }
        }
    }
    
    // 如果所有字体都失败，添加默认字体和基本中文字符
    std::cout << "未找到中文字体文件，使用默认字体..." << std::endl;
    
    // 添加默认字体
    ImFont* default_font = io.Fonts->AddFontDefault();
    
    // 尝试添加基本中文字符到默认字体
    ImFontConfig config;
    config.MergeMode = true;
    config.GlyphMinAdvanceX = 13.0f;
    
    static const ImWchar basic_chinese[] = {
        0x4e00, 0x9fff, // 基本中文字符范围
        0,
    };
    
    // 如果有任何可用的中文字体，尝试合并
    for (const auto& font_path : font_paths) {
        if (std::filesystem::exists(font_path)) {
            io.Fonts->AddFontFromFileTTF(font_path.c_str(), 16.0f, &config, basic_chinese);
            break;
        }
    }
    
    io.Fonts->Build();
    
    std::cout << "字体初始化完成（可能不支持完整中文显示）" << std::endl;
    return false;
}

// 字体回退系统
const char* GUI::getDisplayText(const char* text) {
    // 如果支持中文，直接返回
    if (chinese_font_loaded) {
        return text;
    }
    
    // 否则使用英文映射
    static std::unordered_map<std::string, std::string> text_map = {
        {"文件", "File"},
        {"图像显示", "Image View"},
        {"频域分析", "Frequency Analysis"},
        {"滤波器设置", "Filter Settings"},
        {"统计信息", "Statistics"},
        {"控制面板", "Control Panel"},
        {"关于", "About"},
        {"打开图像", "Open Image"},
        {"保存图像", "Save Image"},
        {"创建测试图像", "Create Test"},
        {"原始图像", "Original"},
        {"频域幅度", "Magnitude"},
        {"频域相位", "Phase"},
        {"滤波后图像", "Filtered"},
        {"对比视图", "Comparison"},
        {"显示模式", "Display Mode"},
        {"颜色映射", "Color Map"},
        {"低通滤波", "Low Pass"},
        {"高通滤波", "High Pass"},
        {"带通滤波", "Band Pass"},
        {"应用滤波器", "Apply Filter"},
        {"重置", "Reset"},
        {"截止频率", "Cutoff Freq"},
        {"最小值", "Min"},
        {"最大值", "Max"},
        {"平均值", "Mean"},
        {"标准差", "Std Dev"}
    };
    
    auto it = text_map.find(text);
    return (it != text_map.end()) ? it->second.c_str() : text;
}
