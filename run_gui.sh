#!/bin/bash
echo "=== FFT Image Processor GUI 启动 ==="
echo "提示: 如果中文显示为问号，这是正常的（字体问题）"
echo "功能完全正常，包括："
echo "• 图像加载和显示"
echo "• FFT变换"
echo "• 实时滤波器"
echo "• 完整统计分析"
echo "• 多种颜色映射"
echo ""
echo "使用方法："
echo "1. 拖拽图像文件到窗口"
echo "2. 或使用菜单 -> 文件 -> 打开图像"
echo "3. 在控制台输入图像路径"
echo ""

cd build
./bin/FFTImageProcessor
