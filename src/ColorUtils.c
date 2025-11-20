#include <stdio.h>
#include <math.h>

// Custom
#include "ColorUtils.h"

// Helper: clamp to 0..1 
static inline double clamp01(double v) { return v > 1.0 ? 1.0 : (v < 0.0 ? 0.0 : v); } 

void val_to_rgb(double val0to1, unsigned char* r, unsigned char* g, unsigned char* b) {
    // Map val in [0,1] to wavelength 400..700 nm and reuse wavelength_to_rgb
    if (!r || !g || !b) return;  
    if (val0to1 < 0.0) val0to1 = 0.0;
    if (val0to1 > 1.0) val0to1 = 1.0;
    // Actual visible ~375–725 nm, but 400–700 is typical to avoid dim ends
    double wavelength = 400.0 + val0to1 * (700.0 - 400.0); 
    wavelength_to_rgb(wavelength, r, g, b);  
}  

void wavelength_to_rgb(double wavelength_nm, unsigned char* r8, unsigned char* g8, unsigned char* b8) {
    if (!r8 || !g8 || !b8) return;  
    double X, Y, Z;
    cie1931_wavelength_to_xyz(wavelength_nm, &X, &Y, &Z); 

    double r, g, b;
    srgb_xyz_to_rgb(X, Y, Z, &r, &g, &b); 

    // Quantize to 8-bit 
    int ri = (int)(r * 255.0 + 0.5);
    int gi = (int)(g * 255.0 + 0.5);
    int bi = (int)(b * 255.0 + 0.5);
    if (ri < 0) ri = 0; else if (ri > 255) ri = 255;
    if (gi < 0) gi = 0; else if (gi > 255) gi = 255;
    if (bi < 0) bi = 0; else if (bi > 255) bi = 255;

    *r8 = (unsigned char)ri;
    *g8 = (unsigned char)gi;
    *b8 = (unsigned char)bi; 
} 

void srgb_xyz_to_rgb(double x, double y, double z, double* r, double* g, double* b) {
    if (!r || !g || !b) return;  
    // sRGB/IEC 61966-2-1 matrix, D65 
    double rl =  3.2406255 * x + (-1.5372080) * y + (-0.4986286) * z;
    double gl = -0.9689307 * x +  1.8757561  * y +  0.0415175  * z;
    double bl =  0.0557101 * x + (-0.2040211) * y +  1.0569959  * z; 

    *r = srgb_xyz_to_rgb_post(rl);
    *g = srgb_xyz_to_rgb_post(gl);
    *b = srgb_xyz_to_rgb_post(bl); 
} 

double srgb_xyz_to_rgb_post(double c) {
    // Clamp and apply sRGB EOTF 
    c = clamp01(c);
    c = (c <= 0.0031308) ? (c * 12.92) : (1.055 * pow(c, 1.0 / 2.4) - 0.055);
    return c; 
} 

void cie1931_wavelength_to_xyz(double wavelength_nm, double* x, double* y, double* z) {
    if (!x || !y || !z) return;
    double wave = wavelength_nm;

    // X 
    {
        double t1 = (wave - 442.0) * ((wave < 442.0) ? 0.0624 : 0.0374);
        double t2 = (wave - 599.8) * ((wave < 599.8) ? 0.0264 : 0.0323);
        double t3 = (wave - 501.1) * ((wave < 501.1) ? 0.0490 : 0.0382);
        *x = 0.362 * exp(-0.5 * t1 * t1)
           + 1.056 * exp(-0.5 * t2 * t2)
           - 0.065 * exp(-0.5 * t3 * t3);
    } 

    // Y 
    {
        double t1 = (wave - 568.8) * ((wave < 568.8) ? 0.0213 : 0.0247);
        double t2 = (wave - 530.9) * ((wave < 530.9) ? 0.0613 : 0.0322);
        *y = 0.821 * exp(-0.5 * t1 * t1)
           + 0.286 * exp(-0.5 * t2 * t2);
    } 

    // Z 
    {
        double t1 = (wave - 437.0) * ((wave < 437.0) ? 0.0845 : 0.0278);
        double t2 = (wave - 459.0) * ((wave < 459.0) ? 0.0385 : 0.0725);
        *z = 1.217 * exp(-0.5 * t1 * t1)
           + 0.681 * exp(-0.5 * t2 * t2);
    } 
} 
