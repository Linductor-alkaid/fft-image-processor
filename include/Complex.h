#ifndef COMPLEX_H
#define COMPLEX_H

#include <cmath>

class Complex {
public:
    double real, imag;
    
    Complex(double r = 0, double i = 0) : real(r), imag(i) {}
    
    Complex operator+(const Complex& other) const {
        return Complex(real + other.real, imag + other.imag);
    }
    
    Complex operator-(const Complex& other) const {
        return Complex(real - other.real, imag - other.imag);
    }
    
    Complex operator*(const Complex& other) const {
        return Complex(real * other.real - imag * other.imag,
                      real * other.imag + imag * other.real);
    }
    
    Complex operator/(double d) const {
        return Complex(real / d, imag / d);
    }
    
    double magnitude() const {
        return sqrt(real * real + imag * imag);
    }
    
    double phase() const {
        return atan2(imag, real);
    }
};

#endif // COMPLEX_H