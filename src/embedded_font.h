#pragma once
#include <vector>

// 基本中文字符映射
const char* basic_chinese_chars = 
    "文件图像频域滤波器统计信息关于打开保存创建测试"
    "原始幅度相位对比灰度低通高通带通应用重置"
    "最小值最大值平均标准差设置类型截止范围"
    "窗口分析控制面板帮助版本功能特性";

// 字符到显示文本的映射
struct ChineseTextMap {
    const char* chinese;
    const char* english;
};

const ChineseTextMap text_mappings[] = {
    {"文件", "File"},
    {"图像", "Image"}, 
    {"频域", "Frequency"},
    {"滤波器", "Filter"},
    {"统计信息", "Statistics"},
    {"关于", "About"},
    {"打开", "Open"},
    {"保存", "Save"},
    {"创建", "Create"},
    {"测试", "Test"},
    {"原始图像", "Original"},
    {"频域幅度", "Magnitude"},
    {"频域相位", "Phase"},
    {"滤波后图像", "Filtered"},
    {"对比视图", "Compare"},
    {"低通滤波", "LowPass"},
    {"高通滤波", "HighPass"},
    {"带通滤波", "BandPass"},
    {"应用滤波器", "Apply Filter"},
    {"重置", "Reset"},
    {"最小值", "Min"},
    {"最大值", "Max"},
    {"平均值", "Mean"},
    {"标准差", "StdDev"},
    {"设置", "Settings"},
    {"截止频率", "Cutoff"},
    {"频率范围", "Range"},
    {"控制面板", "Control"},
    {"显示模式", "Display Mode"},
    {"颜色映射", "Color Map"},
    {nullptr, nullptr}
};

inline const char* get_display_text(const char* chinese_text) {
    for (const auto& mapping : text_mappings) {
        if (mapping.chinese && strcmp(mapping.chinese, chinese_text) == 0) {
            return mapping.english;
        }
    }
    return chinese_text; // fallback to original
}

// 便利宏
#define CTEXT(chinese) get_display_text(chinese)
