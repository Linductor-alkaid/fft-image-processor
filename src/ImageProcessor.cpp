// STB库实现 - 只在这里定义一次
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "ImageProcessor.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <filesystem>

// 跨平台文件对话框
#ifdef _WIN32
    #include <windows.h>
    #include <commdlg.h>
#elif __linux__
    #include <cstdlib>
#elif __APPLE__
    #include <cstdlib>
#endif

const double PI = 3.14159265358979323846;

ImageProcessor::ImageProcessor() : width(0), height(0), originalChannels(0) {
    // 预分配一些合理的容量
    grayImage.reserve(512);
    frequencyDomain.reserve(512);
}

ImageProcessor::~ImageProcessor() {
    // 显式清理大型容器
    grayImage.clear();
    grayImage.shrink_to_fit();
    
    frequencyDomain.clear();
    frequencyDomain.shrink_to_fit();
}

std::string ImageProcessor::openFileDialog() {
    std::string filename;
    
#ifdef _WIN32
    // Windows文件对话框
    OPENFILENAME ofn;
    char szFile[260] = {0};
    
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Image Files\0*.png;*.jpg;*.jpeg;*.bmp;*.tga;*.gif\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    
    if (GetOpenFileName(&ofn)) {
        filename = szFile;
    }
    
#elif __linux__ || __APPLE__
    // Linux/Mac使用zenity或osascript
    #ifdef __linux__
        std::string command = "zenity --file-selection --file-filter='Image files | *.png *.jpg *.jpeg *.bmp *.tga *.gif' --title='Select Image File'";
    #else
        std::string command = "osascript -e 'tell application \"System Events\" to activate' -e 'tell application \"System Events\" to set theFile to choose file with prompt \"Select Image File:\" of type {\"png\", \"jpg\", \"jpeg\", \"bmp\", \"tga\", \"gif\"}' -e 'POSIX path of theFile'";
    #endif
    
    FILE* pipe = popen(command.c_str(), "r");
    if (pipe) {
        char buffer[512];
        if (fgets(buffer, sizeof(buffer), pipe)) {
            filename = buffer;
            // 移除换行符
            filename.erase(std::remove(filename.begin(), filename.end(), '\n'), filename.end());
        }
        pclose(pipe);
    }
#endif
    
    return filename;
}

bool ImageProcessor::loadImageWithDialog() {
    std::string filename = openFileDialog();
    if (filename.empty()) {
        std::cout << "No file selected." << std::endl;
        return false;
    }
    
    return loadImage(filename);
}

bool ImageProcessor::loadImage(const std::string& filename) {
    if (filename.empty()) {
        return loadImageWithDialog();
    }
    
    // 清理之前的数据
    grayImage.clear();
    frequencyDomain.clear();
    
    // 使用 stb_image 加载图像
    unsigned char* imageData = stbi_load(filename.c_str(), &width, &height, &originalChannels, 0);
    
    if (!imageData) {
        std::cerr << "Failed to load image: " << filename << std::endl;
        std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
        width = height = 0;
        return false;
    }
    
    // 检查图像尺寸是否合理
    if (width <= 0 || height <= 0 || width > 4096 || height > 4096) {
        std::cerr << "Image dimensions out of range: " << width << "x" << height << std::endl;
        stbi_image_free(imageData);
        width = height = 0;
        return false;
    }
    
    std::cout << "Successfully loaded: " << filename << std::endl;
    std::cout << "Dimensions: " << width << "x" << height << std::endl;
    std::cout << "Channels: " << originalChannels << std::endl;
    
    try {
        // 转换为灰度图像
        rgbToGray(imageData, width, height, originalChannels);
        
        // 确保图像尺寸是2的幂次
        if (!isPowerOfTwo(width) || !isPowerOfTwo(height) || width != height) {
            std::cout << "Resizing image to power of 2 dimensions for FFT..." << std::endl;
            resizeImageToPowerOfTwo();
        }
        
        printImageInfo();
        
    } catch (const std::exception& e) {
        std::cerr << "Image processing failed: " << e.what() << std::endl;
        grayImage.clear();
        width = height = 0;
        stbi_image_free(imageData);
        return false;
    }
    
    // 释放原始图像数据
    stbi_image_free(imageData);
    
    return true;
}

void ImageProcessor::rgbToGray(unsigned char* imageData, int w, int h, int channels) {
    grayImage.resize(h, std::vector<double>(w));
    
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int pixelIndex = (y * w + x) * channels;
            
            if (channels == 1) {
                // 已经是灰度图
                grayImage[y][x] = static_cast<double>(imageData[pixelIndex]);
            } else if (channels >= 3) {
                // RGB转灰度 (使用标准权重)
                double r = static_cast<double>(imageData[pixelIndex]);
                double g = static_cast<double>(imageData[pixelIndex + 1]);
                double b = static_cast<double>(imageData[pixelIndex + 2]);
                grayImage[y][x] = 0.299 * r + 0.587 * g + 0.114 * b;
            } else {
                // 其他情况，直接使用第一个通道
                grayImage[y][x] = static_cast<double>(imageData[pixelIndex]);
            }
        }
    }
}

bool ImageProcessor::isPowerOfTwo(int n) {
    return n > 0 && (n & (n - 1)) == 0;
}

void ImageProcessor::resizeImageToPowerOfTwo() {
    // 找到最接近的2的幂次，选择正方形
    int newSize = 1;
    int maxDim = std::max(width, height);
    while (newSize < maxDim) {
        newSize *= 2;
    }
    
    // 限制最大尺寸避免内存问题 - 更保守的限制
    if (newSize > 512) {  // 从1024降到512
        newSize = 512;
        std::cout << "Warning: Image too large, limiting to 512x512" << std::endl;
    }
    
    // 检查内存分配是否安全
    size_t requiredMemory = static_cast<size_t>(newSize) * newSize * sizeof(double);
    if (requiredMemory > 500 * 1024 * 1024) { // 500MB限制
        std::cerr << "Error: Image too large for processing" << std::endl;
        return;
    }
    
    try {
        std::vector<std::vector<double>> resizedImage;
        resizedImage.reserve(newSize);
        
        for (int i = 0; i < newSize; i++) {
            resizedImage.emplace_back(newSize, 0.0);
        }
        
        // 使用最近邻插值调整大小
        double scaleX = static_cast<double>(width) / newSize;
        double scaleY = static_cast<double>(height) / newSize;
        
        for (int y = 0; y < newSize; y++) {
            for (int x = 0; x < newSize; x++) {
                int srcX = static_cast<int>(x * scaleX);
                int srcY = static_cast<int>(y * scaleY);
                
                // 边界检查
                srcX = std::clamp(srcX, 0, width - 1);
                srcY = std::clamp(srcY, 0, height - 1);
                
                resizedImage[y][x] = grayImage[srcY][srcX];
            }
        }
        
        // 安全地替换原图像
        grayImage = std::move(resizedImage);
        width = height = newSize;
        
        std::cout << "Image resized to: " << newSize << "x" << newSize << std::endl;
        
    } catch (const std::bad_alloc& e) {
        std::cerr << "Memory allocation failed: " << e.what() << std::endl;
        return;
    }
}

void ImageProcessor::createTestImage(int size) {
    width = height = size;
    grayImage.resize(height, std::vector<double>(width));
    
    // 创建更复杂的测试图像
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // 棋盘格 + 正弦波模式
            double checkerboard = ((x / 32 + y / 32) % 2 == 0) ? 255.0 : 0.0;
            double sine_wave = 127.5 * (1.0 + sin(2 * PI * x / 64.0) * sin(2 * PI * y / 64.0));
            grayImage[y][x] = (checkerboard + sine_wave) / 2.0;
        }
    }
    
    originalChannels = 1;
    std::cout << "Test image created: " << width << "x" << height << std::endl;
}

int ImageProcessor::bitReverse(int n, int bits) {
    int result = 0;
    for (int i = 0; i < bits; i++) {
        result = (result << 1) | (n & 1);
        n >>= 1;
    }
    return result;
}

void ImageProcessor::fft1D(std::vector<Complex>& data) {
    int n = data.size();
    
    // 检查输入数据的有效性
    for (int i = 0; i < n; i++) {
        if (!std::isfinite(data[i].real) || !std::isfinite(data[i].imag)) {
            std::cerr << "Warning: Invalid input data at index " << i << std::endl;
            data[i] = Complex(0, 0);
        }
    }
    
    if (n <= 1) return;
    
    // 确保n是2的幂次
    if (!isPowerOfTwo(n)) {
        std::cerr << "Error: FFT size must be power of 2, got " << n << std::endl;
        return;
    }
    
    int bits = static_cast<int>(log2(n));
    
    // 位反转重排
    for (int i = 0; i < n; i++) {
        int j = bitReverse(i, bits);
        if (i < j) {
            std::swap(data[i], data[j]);
        }
    }
    
    // 蝶形运算
    for (int len = 2; len <= n; len *= 2) {
        double angle = -2.0 * PI / len;
        Complex wlen(cos(angle), sin(angle));
        
        for (int i = 0; i < n; i += len) {
            Complex w(1.0, 0.0);
            for (int j = 0; j < len / 2; j++) {
                Complex u = data[i + j];
                Complex v = data[i + j + len / 2] * w;
                
                data[i + j] = u + v;
                data[i + j + len / 2] = u - v;
                
                w = w * wlen;
                
                // 检查中间结果的数值稳定性
                if (!std::isfinite(data[i + j].real) || !std::isfinite(data[i + j].imag) ||
                    !std::isfinite(data[i + j + len / 2].real) || !std::isfinite(data[i + j + len / 2].imag)) {
                    std::cerr << "Warning: Numerical instability detected in FFT" << std::endl;
                }
            }
        }
    }
}


void ImageProcessor::ifft1D(std::vector<Complex>& data) {
    int n = data.size();
    
    // 共轭
    for (auto& c : data) {
        c.imag = -c.imag;
    }
    
    fft1D(data);
    
    // 共轭并归一化
    for (auto& c : data) {
        c.imag = -c.imag;
        c = c / n;
    }
}

void ImageProcessor::fft2D() {if (grayImage.empty()) {
        std::cerr << "No image loaded for FFT processing!" << std::endl;
        return;
    }
    
    // 检查图像尺寸
    if (width <= 0 || height <= 0 || width > 1024 || height > 1024) {
        std::cerr << "Invalid image dimensions for FFT: " << width << "x" << height << std::endl;
        return;
    }
    
    try {
        // 清理之前的频域数据
        frequencyDomain.clear();
        frequencyDomain.resize(height, std::vector<Complex>(width));
        
        // 初始化频域数据 - 确保数据完整性
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                // 检查边界
                if (y < grayImage.size() && x < grayImage[y].size()) {
                    frequencyDomain[y][x] = Complex(grayImage[y][x], 0.0);
                } else {
                    frequencyDomain[y][x] = Complex(0.0, 0.0);
                }
            }
        }
        
        std::cout << "开始FFT处理..." << std::endl;
        
        // 对每一行进行FFT
        for (int y = 0; y < height; y++) {
            fft1D(frequencyDomain[y]);
        }
        
        // 对每一列进行FFT
        for (int x = 0; x < width; x++) {
            std::vector<Complex> column(height);
            for (int y = 0; y < height; y++) {
                column[y] = frequencyDomain[y][x];
            }
            fft1D(column);
            for (int y = 0; y < height; y++) {
                frequencyDomain[y][x] = column[y];
            }
        }
        
        std::cout << "2D FFT completed successfully" << std::endl;

        frequencyDomain = fftShift(frequencyDomain);
        
        // 立即分析结果
        analyzeFrequencySpectrum();
        
    } catch (const std::exception& e) {
        std::cerr << "FFT processing failed: " << e.what() << std::endl;
        frequencyDomain.clear();
    }
}

std::vector<std::vector<double>> ImageProcessor::ifft2D(const std::vector<std::vector<Complex>>& freqData) {
    if (freqData.empty()) {
        std::cerr << "Empty frequency data for IFFT!" << std::endl;
        return std::vector<std::vector<double>>();
    }
    
    std::vector<std::vector<Complex>> temp = ifftShift(freqData);
    
    try {
        // 对每一行进行IFFT
        for (int y = 0; y < height; y++) {
            if (y < temp.size()) {
                ifft1D(temp[y]);
            }
        }
        
        // 对每一列进行IFFT
        for (int x = 0; x < width; x++) {
            std::vector<Complex> column(height);
            for (int y = 0; y < height; y++) {
                if (y < temp.size() && x < temp[y].size()) {
                    column[y] = temp[y][x];
                }
            }
            ifft1D(column);
            for (int y = 0; y < height; y++) {
                if (y < temp.size() && x < temp[y].size()) {
                    temp[y][x] = column[y];
                }
            }
        }
        
        // 提取实部并正确处理幅度
        std::vector<std::vector<double>> result(height, std::vector<double>(width));
        
        // 计算结果的统计信息用于调试
        double minVal = 1e9, maxVal = -1e9, sum = 0;
        int count = 0;
        
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (y < temp.size() && x < temp[y].size()) {
                    double realPart = temp[y][x].real;
                    
                    // 检查数值有效性
                    if (std::isfinite(realPart)) {
                        minVal = std::min(minVal, realPart);
                        maxVal = std::max(maxVal, realPart);
                        sum += realPart;
                        count++;
                        
                        // 将值限制在合理范围内
                        result[y][x] = std::max(0.0, std::min(255.0, realPart));
                    } else {
                        result[y][x] = 0.0;
                    }
                } else {
                    result[y][x] = 0.0;
                }
            }
        }
        
        if (count > 0) {
            double mean = sum / count;
            std::cout << "IFFT result - Min: " << minVal 
                      << ", Max: " << maxVal 
                      << ", Mean: " << mean << std::endl;
            
            // 如果值域不在正常范围内，进行重新归一化
            if (maxVal > 1000 || minVal < -100) {
                std::cout << "Renormalizing IFFT result..." << std::endl;
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        if (y < temp.size() && x < temp[y].size()) {
                            double normalizedVal = (temp[y][x].real - minVal) / (maxVal - minVal) * 255.0;
                            result[y][x] = std::max(0.0, std::min(255.0, normalizedVal));
                        }
                    }
                }
            }
        }
        
        return result;
        
    } catch (const std::exception& e) {
        std::cerr << "IFFT processing failed: " << e.what() << std::endl;
        return std::vector<std::vector<double>>(height, std::vector<double>(width, 0.0));
    }
}


std::vector<std::vector<Complex>> ImageProcessor::lowPassFilter(double cutoffRatio) {
    if (frequencyDomain.empty()) {
        std::cerr << "No frequency domain data available for filtering!" << std::endl;
        return std::vector<std::vector<Complex>>();
    }
    
    std::vector<std::vector<Complex>> filtered = frequencyDomain;
    
    int centerX = width / 2;
    int centerY = height / 2;
    double maxRadius = sqrt(width * width + height * height) / 2.0 * cutoffRatio;
    
    std::cout << "Low-pass filter: cutoff=" << cutoffRatio 
              << ", radius=" << maxRadius 
              << ", center=(" << centerX << "," << centerY << ")" << std::endl;
    
    // 遍历整个频域
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // 计算到频域中心的距离，考虑FFT的周期性
            
            double dx = double(x) - centerX;
            double dy = double(y) - centerY;
            double distance = sqrt(dx * dx + dy * dy);
            
            // 如果距离超过截止半径，则将该频率分量置零
            if (distance > maxRadius) {
                filtered[y][x] = Complex(0, 0);
            }
        }
    }
    
    return filtered;
}

std::vector<std::vector<Complex>> ImageProcessor::highPassFilter(double cutoffRatio) {
    if (frequencyDomain.empty()) {
        std::cerr << "No frequency domain data available for filtering!" << std::endl;
        return std::vector<std::vector<Complex>>();
    }
    
    std::vector<std::vector<Complex>> filtered = frequencyDomain;
    
    int centerX = width / 2;
    int centerY = height / 2;
    double minRadius = sqrt(width * width + height * height) / 2.0 * cutoffRatio;
    
    std::cout << "High-pass filter: cutoff=" << cutoffRatio 
              << ", radius=" << minRadius 
              << ", center=(" << centerX << "," << centerY << ")" << std::endl;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            
            double dx = double(x) - centerX;
            double dy = double(y) - centerY;
            double distance = sqrt(dx * dx + dy * dy);
            
            // 如果距离小于截止半径，则将该频率分量置零
            if (distance < minRadius) {
                filtered[y][x] = Complex(0, 0);
            }
        }
    }
    
    return filtered;
}

std::vector<std::vector<Complex>> ImageProcessor::bandPassFilter(double lowCutoff, double highCutoff) {
    if (frequencyDomain.empty()) {
        std::cerr << "No frequency domain data available for filtering!" << std::endl;
        return std::vector<std::vector<Complex>>();
    }
    
    std::vector<std::vector<Complex>> filtered = frequencyDomain;
    
    int centerX = width / 2;
    int centerY = height / 2;
    double minRadius = sqrt(width * width + height * height) / 2.0 * lowCutoff;
    double maxRadius = sqrt(width * width + height * height) / 2.0 * highCutoff;
    
    std::cout << "Band-pass filter: low=" << lowCutoff 
              << ", high=" << highCutoff 
              << ", radius range=[" << minRadius << "," << maxRadius << "]" << std::endl;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            
            double dx = double(x) - centerX;
            double dy = double(y) - centerY;
            double distance = sqrt(dx * dx + dy * dy);
            
            // 保留在指定频率范围内的分量
            if (distance < minRadius || distance > maxRadius) {
                filtered[y][x] = Complex(0, 0);
            }
        }
    }
    
    return filtered;
}

std::vector<std::vector<Complex>> ImageProcessor::lowPassFilterCentered(double cutoffRatio) {
    if (frequencyDomain.empty()) {
        std::cerr << "No frequency domain data available for filtering!" << std::endl;
        return std::vector<std::vector<Complex>>();
    }
    
    // 先进行FFT移位，将低频移到中心
    auto centeredFreq = fftShift(frequencyDomain);
    
    int centerX = width / 2;
    int centerY = height / 2;
    double maxRadius = std::min(width, height) / 2.0 * cutoffRatio;
    
    // 在中心化的频域中应用滤波
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double dx = x - centerX;
            double dy = y - centerY;
            double distance = sqrt(dx * dx + dy * dy);
            
            if (distance > maxRadius) {
                centeredFreq[y][x] = Complex(0, 0);
            }
        }
    }
    
    // 移位回原来的位置
    return ifftShift(centeredFreq);
}

// FFT移位函数（将频域中心化）
std::vector<std::vector<Complex>> ImageProcessor::fftShift(const std::vector<std::vector<Complex>>& input) {
    int rows = input.size();
    int cols = input[0].size();
    std::vector<std::vector<Complex>> shifted(rows, std::vector<Complex>(cols));
    
    int halfRows = rows / 2;
    int halfCols = cols / 2;
    
    // 四个象限的交换
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            int newY = (y + halfRows) % rows;
            int newX = (x + halfCols) % cols;
            shifted[newY][newX] = input[y][x];
        }
    }
    
    return shifted;
}

// 逆FFT移位函数
std::vector<std::vector<Complex>> ImageProcessor::ifftShift(const std::vector<std::vector<Complex>>& input) {
    // ifftShift与fftShift是相同的操作（对于偶数尺寸）
    return fftShift(input);
}

double ImageProcessor::calculateMSE(const std::vector<std::vector<double>>& img1,
                                   const std::vector<std::vector<double>>& img2) {
    double mse = 0.0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double diff = img1[y][x] - img2[y][x];
            mse += diff * diff;
        }
    }
    return mse / (width * height);
}

double ImageProcessor::calculatePSNR(double mse) {
    if (mse == 0) return 100.0; // Perfect match
    return 10 * log10((255.0 * 255.0) / mse);
}

double ImageProcessor::calculateSSIM(const std::vector<std::vector<double>>& img1,
                                    const std::vector<std::vector<double>>& img2) {
    // 简化的SSIM计算
    double mean1 = 0, mean2 = 0;
    double var1 = 0, var2 = 0, cov = 0;
    int pixels = width * height;
    
    // 计算均值
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            mean1 += img1[y][x];
            mean2 += img2[y][x];
        }
    }
    mean1 /= pixels;
    mean2 /= pixels;
    
    // 计算方差和协方差
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double diff1 = img1[y][x] - mean1;
            double diff2 = img2[y][x] - mean2;
            var1 += diff1 * diff1;
            var2 += diff2 * diff2;
            cov += diff1 * diff2;
        }
    }
    var1 /= pixels - 1;
    var2 /= pixels - 1;
    cov /= pixels - 1;
    
    // SSIM计算
    double c1 = 6.5025, c2 = 58.5225; // 常数
    double numerator = (2 * mean1 * mean2 + c1) * (2 * cov + c2);
    double denominator = (mean1 * mean1 + mean2 * mean2 + c1) * (var1 + var2 + c2);
    
    return numerator / denominator;
}

std::vector<float> ImageProcessor::getFrequencyVisualizationData() {
    std::vector<float> vertices;
    
    if (frequencyDomain.empty() || height <= 0 || width <= 0) {
        std::cerr << "No frequency domain data available" << std::endl;
        return vertices;
    }
    
    // 计算采样步长，确保不会产生过多数据点
    int stepSize = std::max(1, std::min(width, height) / 128); // 限制最多128x128个点
    int expectedPoints = (height / stepSize) * (width / stepSize);
    
    if (expectedPoints > 50000) { // 限制最大点数
        stepSize = std::max(2, static_cast<int>(sqrt(width * height / 25000.0)));
    }
    
    try {
        vertices.reserve(expectedPoints * 6); // 预分配内存
        
        for (int y = 0; y < height; y += stepSize) {
            for (int x = 0; x < width; x += stepSize) {
                // 边界检查
                if (y >= height || x >= width) continue;
                
                // 位置 (归一化到[-1,1])
                float posX = (float)x / width * 2.0f - 1.0f;
                float posY = (float)y / height * 2.0f - 1.0f;
                
                // 幅度（对数处理）
                double magnitude = frequencyDomain[y][x].magnitude();
                if (!std::isfinite(magnitude)) magnitude = 0.0;
                
                float posZ = static_cast<float>(log(1.0 + magnitude) * 0.02f); // 更保守的缩放
                
                // 相位转换为颜色
                double phase = frequencyDomain[y][x].phase();
                if (!std::isfinite(phase)) phase = 0.0;
                
                float r = static_cast<float>((sin(phase) + 1.0) * 0.5);
                float g = static_cast<float>((cos(phase) + 1.0) * 0.5);
                float b = static_cast<float>((sin(phase + PI/2) + 1.0) * 0.5);
                
                vertices.insert(vertices.end(), {posX, posY, posZ, r, g, b});
            }
        }
        
        std::cout << "Generated " << (vertices.size() / 6) << " visualization points" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Visualization data generation failed: " << e.what() << std::endl;
        vertices.clear();
    }
    
    return vertices;
}

bool ImageProcessor::saveImage(const std::string& filename, const std::vector<std::vector<double>>& image) {
    // 准备图像数据
    std::vector<unsigned char> imageData(width * height);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            imageData[y * width + x] = static_cast<unsigned char>(
                std::max(0.0, std::min(255.0, image[y][x]))
            );
        }
    }
    
    // 根据文件扩展名选择格式
    std::string ext = filename.substr(filename.find_last_of('.'));
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    int result = 0;
    if (ext == ".png") {
        result = stbi_write_png(filename.c_str(), width, height, 1, imageData.data(), width);
    } else if (ext == ".jpg" || ext == ".jpeg") {
        result = stbi_write_jpg(filename.c_str(), width, height, 1, imageData.data(), 90);
    } else if (ext == ".bmp") {
        result = stbi_write_bmp(filename.c_str(), width, height, 1, imageData.data());
    } else if (ext == ".tga") {
        result = stbi_write_tga(filename.c_str(), width, height, 1, imageData.data());
    } else {
        std::cerr << "Unsupported image format: " << ext << std::endl;
        return false;
    }
    
    if (result) {
        std::cout << "Image saved successfully: " << filename << std::endl;
        return true;
    } else {
        std::cerr << "Failed to save image: " << filename << std::endl;
        return false;
    }
}

void ImageProcessor::printImageInfo() const {
    std::cout << "\n=== Image Information ===" << std::endl;
    std::cout << "Dimensions: " << width << "x" << height << std::endl;
    std::cout << "Original channels: " << originalChannels << std::endl;
    std::cout << "Total pixels: " << (width * height) << std::endl;
    
    if (!grayImage.empty()) {
        // 计算统计信息
        double minVal = 255.0, maxVal = 0.0, sum = 0.0;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                double val = grayImage[y][x];
                minVal = std::min(minVal, val);
                maxVal = std::max(maxVal, val);
                sum += val;
            }
        }
        double mean = sum / (width * height);
        
        std::cout << "Pixel value range: [" << minVal << ", " << maxVal << "]" << std::endl;
        std::cout << "Mean pixel value: " << mean << std::endl;
    }
    std::cout << "=========================" << std::endl;
}

void ImageProcessor::analyzeFrequencyDomain() const {
    if (frequencyDomain.empty()) {
        std::cout << "No frequency domain data to analyze" << std::endl;
        return;
    }
    
    double maxMagnitude = 0;
    int maxX = 0, maxY = 0;
    double totalEnergy = 0;
    
    std::cout << "\n=== Frequency Domain Analysis ===" << std::endl;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double magnitude = frequencyDomain[y][x].magnitude();
            totalEnergy += magnitude * magnitude;
            
            if (magnitude > maxMagnitude) {
                maxMagnitude = magnitude;
                maxX = x;
                maxY = y;
            }
        }
    }
    
    std::cout << "Max magnitude: " << maxMagnitude 
              << " at (" << maxX << "," << maxY << ")" << std::endl;
    std::cout << "Total energy: " << totalEnergy << std::endl;
    
    // 检查DC分量（应该在(0,0)）
    double dcMagnitude = frequencyDomain[0][0].magnitude();
    std::cout << "DC component magnitude: " << dcMagnitude << std::endl;
    std::cout << "DC component real: " << frequencyDomain[0][0].real 
              << ", imag: " << frequencyDomain[0][0].imag << std::endl;
    std::cout << "=================================" << std::endl;
}

void ImageProcessor::analyzeFrequencySpectrum() const {
    if (frequencyDomain.empty()) {
        std::cout << "No frequency domain data to analyze" << std::endl;
        return;
    }
    
    std::cout << "\n=== 详细频域分析 ===" << std::endl;
    
    // 检查关键位置的幅度值
    std::cout << "关键位置幅度值：" << std::endl;
    std::cout << "DC分量 (0,0): " << frequencyDomain[0][0].magnitude() << std::endl;
    std::cout << "右上角 (0," << (width-1) << "): " << frequencyDomain[0][width-1].magnitude() << std::endl;
    std::cout << "左下角 (" << (height-1) << ",0): " << frequencyDomain[height-1][0].magnitude() << std::endl;
    std::cout << "右下角 (" << (height-1) << "," << (width-1) << "): " << frequencyDomain[height-1][width-1].magnitude() << std::endl;
    std::cout << "中心 (" << (height/2) << "," << (width/2) << "): " << frequencyDomain[height/2][width/2].magnitude() << std::endl;
    
    // 分析不同区域的平均幅度
    double cornerSum = 0, centerSum = 0, edgeSum = 0;
    int cornerCount = 0, centerCount = 0, edgeCount = 0;
    
    int quarterW = width / 4;
    int quarterH = height / 4;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double magnitude = frequencyDomain[y][x].magnitude();
            
            // 角落区域 (低频)
            if ((x < quarterW || x >= width - quarterW) && 
                (y < quarterH || y >= height - quarterH)) {
                cornerSum += magnitude;
                cornerCount++;
            }
            // 中心区域 (高频)
            else if (x >= quarterW && x < width - quarterW && 
                     y >= quarterH && y < height - quarterH) {
                centerSum += magnitude;
                centerCount++;
            }
            // 边缘区域 (中频)
            else {
                edgeSum += magnitude;
                edgeCount++;
            }
        }
    }
    
    std::cout << "区域平均幅度：" << std::endl;
    std::cout << "角落(低频): " << (cornerSum / cornerCount) << std::endl;
    std::cout << "边缘(中频): " << (edgeSum / edgeCount) << std::endl;
    std::cout << "中心(高频): " << (centerSum / centerCount) << std::endl;
    
    // 检查原始图像的特性
    double imageSum = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            imageSum += grayImage[y][x];
        }
    }
    double imageMean = imageSum / (width * height);
    std::cout << "原图平均亮度: " << imageMean << std::endl;
    std::cout << "理论DC分量: " << (imageMean * width * height) << std::endl;
    
    std::cout << "=====================" << std::endl;
}