#!/bin/bash

echo "=== FFT Image Processor 依赖库安装脚本 ==="
echo "- 频域可视化和分析"检查是否在项目根目录
if [ ! -f "CMakeLists.txt" ]; then
    echo "错误: 请在项目根目录运行此脚本"
    exit 1
fi

# 检查系统依赖
echo "检查系统依赖..."

# 检测操作系统
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo "检测到Linux系统"
    echo "请确保安装了以下依赖包："
    echo "sudo apt update"
    echo "sudo apt install cmake build-essential"
    echo "sudo apt install libglfw3-dev libglew-dev libgtk-3-dev"
    echo ""
    
    # 检查是否安装了必要的包
    check_package() {
        if dpkg -l | grep -q "^ii  $1 "; then
            echo "✓ $1 已安装"
        else
            echo "✗ $1 未安装"
            MISSING_PACKAGES="$MISSING_PACKAGES $1"
        fi
    }
    
    MISSING_PACKAGES=""
    check_package "cmake"
    check_package "build-essential"
    check_package "libglfw3-dev"
    check_package "libglew-dev"
    check_package "libgtk-3-dev"
    
    if [ ! -z "$MISSING_PACKAGES" ]; then
        echo ""
        echo "发现缺少的包，正在安装..."
        sudo apt update
        sudo apt install -y $MISSING_PACKAGES
    fi
    
elif [[ "$OSTYPE" == "darwin"* ]]; then
    echo "检测到macOS系统"
    if command -v brew &> /dev/null; then
        echo "正在通过Homebrew安装依赖..."
        brew install cmake glfw glew
    else
        echo "请先安装Homebrew: https://brew.sh/"
        echo "然后运行: brew install cmake glfw glew"
        exit 1
    fi
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
    echo "检测到Windows系统"
    echo "请使用vcpkg安装依赖:"
    echo "vcpkg install glfw3 glew"
    exit 1
fi

echo ""
echo "=== 项目结构验证 ==="

# 验证文件结构
check_file() {
    if [ -f "$1" ]; then
        echo "✓ $1"
    else
        echo "✗ $1 (缺失)"
    fi
}

check_dir() {
    if [ -d "$1" ]; then
        echo "✓ $1/"
    else
        echo "✗ $1/ (缺失)"
    fi
}

echo "检查项目文件:"
check_file "CMakeLists.txt"
check_file "main.cpp"
check_dir "include"
check_dir "src"
check_dir "third_party"

echo ""
echo "检查依赖库:"
check_dir "third_party/imgui"
check_file "third_party/imgui/imgui.h"
check_file "third_party/imgui/backends/imgui_impl_glfw.h"
check_file "third_party/imgui/backends/imgui_impl_opengl3.h"
check_dir "third_party/implot"
check_file "third_party/implot/implot.h"

echo ""
echo "=== 编译测试 ==="

mkdir -p third_party
cd third_party

# 下载并设置ImGui
echo "正在下载ImGui..."
if [ ! -d "imgui" ]; then
    git clone https://github.com/ocornut/imgui.git
    cd imgui
    git checkout v1.89.2
    echo "ImGui下载完成"
    cd ..
else
    echo "ImGui已存在，跳过下载"
fi

# 下载并设置ImPlot
echo "正在下载ImPlot..."
if [ ! -d "implot" ]; then
    git clone https://github.com/epezent/implot.git
    cd implot
    git checkout v0.14
    echo "ImPlot下载完成"
    cd ..
else
    echo "ImPlot已存在，跳过下载"
fi

# 下载并设置Native File Dialog
echo "正在下载Native File Dialog..."
if [ ! -d "nativefiledialog" ]; then
    git clone https://github.com/mlabbe/nativefiledialog.git
    echo "Native File Dialog下载完成"
else
    echo "Native File Dialog已存在，跳过下载"
fi

# 回到项目根目录
cd ..

echo ""
echo "=== 编译测试 ==="

# 创建构建目录
mkdir -p build
cd build

# 配置项目
echo "正在配置项目..."
if cmake ..; then
    echo "✓ CMake配置成功"
    
    # 尝试编译
    echo "正在编译项目..."
    if make -j$(nproc); then
        echo "✓ 编译成功!"
        echo ""
        echo "项目已成功构建，可执行文件位于: build/bin/FFTImageProcessor"
        echo "运行命令: ./bin/FFTImageProcessor"
    else
        echo "✗ 编译失败，请检查错误信息"
        exit 1
    fi
else
    echo "✗ CMake配置失败，请检查依赖安装"
    exit 1
fi

cd ..

echo ""
echo "=== 安装完成 ==="
echo "所有依赖已安装完成!"
echo "如果遇到问题，请检查:"
echo "1. 系统依赖是否正确安装"
echo "2. 第三方库是否完整下载"
echo "3. 编译错误信息"
echo ""
chmod +x run_gui.sh
echo "运行项目: ./run_gui.sh"