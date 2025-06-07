#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include <vector>
#include <string>
#include "Complex.h"

// 前向声明，避免在头文件中包含实现
extern "C" {
    unsigned char *stbi_load(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);
    void stbi_image_free(void *retval_from_stbi_load);
    char const *stbi_failure_reason(void);
    
    int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_bytes);
    int stbi_write_bmp(char const *filename, int w, int h, int comp, const void *data);
    int stbi_write_tga(char const *filename, int w, int h, int comp, const void *data);
    int stbi_write_jpg(char const *filename, int w, int h, int comp, const void *data, int quality);
}

class ImageProcessor {
private:
    std::vector<std::vector<double>> grayImage;
    std::vector<std::vector<Complex>> frequencyDomain;
    int width, height;
    int originalChannels; // 原始图像通道数
    
    // 位反转函数
    int bitReverse(int n, int bits);
    
    // 1D FFT/IFFT实现
    void fft1D(std::vector<Complex>& data);
    void ifft1D(std::vector<Complex>& data);
    
    // 辅助函数
    void rgbToGray(unsigned char* imageData, int w, int h, int channels);
    std::string openFileDialog();
    bool isPowerOfTwo(int n);
    void resizeImageToPowerOfTwo();

    // FFT移位相关函数
    std::vector<std::vector<Complex>> fftShift(const std::vector<std::vector<Complex>>& input);
    std::vector<std::vector<Complex>> ifftShift(const std::vector<std::vector<Complex>>& input);

    void analyzeFrequencySpectrum() const;

public:
    // 构造函数
    ImageProcessor();
    ~ImageProcessor();
    
    // 图像加载相关
    bool loadImage(const std::string& filename = "");
    bool loadImageWithDialog(); // 弹出文件选择对话框
    void createTestImage(int size = 256); // 创建测试图像
    
    // 2D FFT变换
    void fft2D();
    
    // 2D 逆FFT变换
    std::vector<std::vector<double>> ifft2D(const std::vector<std::vector<Complex>>& freqData);
    
    // 滤波器
    std::vector<std::vector<Complex>> lowPassFilter(double cutoffRatio);
    std::vector<std::vector<Complex>> highPassFilter(double cutoffRatio);
    std::vector<std::vector<Complex>> bandPassFilter(double lowCutoff, double highCutoff);
    std::vector<std::vector<Complex>> lowPassFilterCentered(double cutoffRatio);
    
    // 性能指标计算
    double calculateMSE(const std::vector<std::vector<double>>& img1,
                       const std::vector<std::vector<double>>& img2);
    double calculatePSNR(double mse);
    double calculateSSIM(const std::vector<std::vector<double>>& img1,
                        const std::vector<std::vector<double>>& img2);
    
    // 3D可视化数据生成
    std::vector<float> getFrequencyVisualizationData();
    
    // 图像保存
    bool saveImage(const std::string& filename, const std::vector<std::vector<double>>& image);
    
    // Getter方法
    const std::vector<std::vector<double>>& getGrayImage() const { return grayImage; }
    const std::vector<std::vector<Complex>>& getFrequencyDomain() const { return frequencyDomain; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getOriginalChannels() const { return originalChannels; }
    
    // 信息输出
    void printImageInfo() const;

    // 调试和分析方法
    void analyzeFrequencyDomain() const;
};

#endif // IMAGE_PROCESSOR_H