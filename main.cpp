#include <iostream>
#include "include/GUI.h"

int main() {
    std::cout << "=== FFT Image Processor - GUI Version ===" << std::endl;
    std::cout << "Initializing graphical interface..." << std::endl;
    
    // 创建GUI实例
    GUI gui;
    
    // 初始化GUI
    if (!gui.initialize()) {
        std::cerr << "Failed to initialize GUI" << std::endl;
        return -1;
    }
    
    std::cout << "GUI initialized successfully!" << std::endl;
    std::cout << "=== 使用说明 ===" << std::endl;
    std::cout << "• 拖拽图像文件到窗口中直接加载" << std::endl;
    std::cout << "• 使用菜单栏进行各种操作" << std::endl;
    std::cout << "• 所有窗口都可以拖拽和调整大小" << std::endl;
    std::cout << "• 支持多种颜色映射和显示模式" << std::endl;
    std::cout << "• 实时滤波器预览和参数调整" << std::endl;
    std::cout << "========================" << std::endl;
    
    // 运行主循环
    gui.run();
    
    // 清理资源
    gui.cleanup();
    
    std::cout << "Program completed successfully!" << std::endl;
    return 0;
}