#ifndef COLORUTILS_C_H_
#define COLORUTILS_C_H_

// Map a normalized scalar in [0,1] to RGB (implementation based on calling wavelength mapping 400–700 nm)
void val_to_rgb(double val0to1, unsigned char* r, unsigned char* g, unsigned char* b); 

// Convert wavelength in nm to 8-bit sRGB
void wavelength_to_rgb(double wavelength_nm, unsigned char* r, unsigned char* g, unsigned char* b); 

// Convert CIE XYZ (D65, normalized to 0..1) to linear sRGB with gamma and clamp to 0..1
void srgb_xyz_to_rgb(double x, double y, double z, double* r, double* g, double* b); 

// Postprocess a single sRGB channel: clamp and apply EOTF
double srgb_xyz_to_rgb_post(double c); 

// CIE 1931 2° CMFs (Wyman et al. analytic fit): wavelength (nm) to XYZ, 0..1
void cie1931_wavelength_to_xyz(double wavelength_nm, double* x, double* y, double* z); 

#endif
