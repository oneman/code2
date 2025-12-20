#include <png.h>  
#include <stdio.h>  
#include <stdlib.h>  
 
// Struct to hold image data  
typedef struct {  
    unsigned char *data;  // Pixel data (RGBA8888)  
    int width;            // Image width  
    int height;           // Image height  
    int stride;           // Bytes per row (width * 4 for RGBA)  
} PNGImage;  
 
// Load PNG file into RGBA8888 format  
PNGImage load_png(const char *filename) {  
    PNGImage img = {NULL, 0, 0, 0};  
    FILE *fp = fopen(filename, "rb");  
    if (!fp) {  
        fprintf(stderr, "Error: Could not open file %s\n", filename);  
        return img;  
    }  
 
    // Check PNG signature (first 8 bytes)  
    unsigned char sig[8];  
    fread(sig, 1, 8, fp);  
    if (!png_check_sig(sig, 8)) {  
        fprintf(stderr, "Error: %s is not a valid PNG\n", filename);  
        fclose(fp);  
        return img;  
    }  
png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);  
if (!png_ptr) {  
    fprintf(stderr, "Error: png_create_read_struct failed\n");  
    fclose(fp);  
    return img;  
}  
 
png_infop info_ptr = png_create_info_struct(png_ptr);  
if (!info_ptr) {  
    fprintf(stderr, "Error: png_create_info_struct failed\n");  
    png_destroy_read_struct(&png_ptr, NULL, NULL);  
    fclose(fp);  
    return img;  
}  
 
// Set up error handling (libpng uses setjmp/longjmp)  
if (setjmp(png_jmpbuf(png_ptr))) {  
    fprintf(stderr, "Error: PNG decoding failed\n");  
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);  
    fclose(fp);  
    free(img.data);  
    return img;  
}  
 
// Initialize PNG reading  
png_init_io(png_ptr, fp);  
png_set_sig_bytes(png_ptr, 8);  // We already read the 8-byte signature  
 
// Read PNG metadata (width, height, color type, etc.)  
png_read_info(png_ptr, info_ptr);  
img.width = png_get_image_width(png_ptr, info_ptr);  
img.height = png_get_image_height(png_ptr, info_ptr);  
png_byte color_type = png_get_color_type(png_ptr, info_ptr);  
png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);  
// ... (continue to 4.3)  

if (color_type == PNG_COLOR_TYPE_PALETTE)  
    png_set_palette_to_rgb(png_ptr);  
if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)  
    png_set_expand_gray_1_2_4_to_8(png_ptr);  
if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))  
    png_set_tRNS_to_alpha(png_ptr);  // Convert transparency to alpha channel  
if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE)  
    png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);  // Add alpha channel (0xFF = opaque)  
if (bit_depth == 16)  
    png_set_scale_16(png_ptr);  // Downscale 16-bit to 8-bit  
 
// Update info after conversions  
png_read_update_info(png_ptr, info_ptr);  
 
// Get row bytes and allocate pixel data  
img.stride = png_get_rowbytes(png_ptr, info_ptr);  // Should be width * 4 (RGBA)  
img.data = malloc(img.stride * img.height);  
if (!img.data) {  
    fprintf(stderr, "Error: Failed to allocate pixel buffer\n");  
    // Cleanup and return  
}  
 
// Read pixel rows (libpng requires an array of row pointers)  
png_bytep *row_pointers = malloc(sizeof(png_bytep) * img.height);  
for (int y = 0; y < img.height; y++)  
    row_pointers[y] = img.data + y * img.stride;  
 
png_read_image(png_ptr, row_pointers);  
 
// Cleanup  
free(row_pointers);  
png_destroy_read_struct(&png_ptr, &info_ptr, NULL);  
fclose(fp);
  return img;  
}
