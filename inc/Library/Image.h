#ifndef IMAGE_H
#define IMAGE_H

#if defined __linux__

#include "Files.h"

#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <jpeglib.h>
//#include <zlib.h>
// -lz -lpng -ljpeg

typedef struct {
    const unsigned char* data;
    size_t size;
    size_t offset;
} PngMemReader;

void Png_Read(png_structp png_ptr, png_bytep out_bytes, png_size_t byte_count) {
    PngMemReader* reader = (PngMemReader*)png_get_io_ptr(png_ptr);
    if (reader->offset + byte_count > reader->size) {
        png_error(png_ptr, "Read beyond buffer");
    }
    memcpy(out_bytes, reader->data + reader->offset, byte_count);
    reader->offset += byte_count;
}
int Png_SaveARGB(const char* filename,unsigned int* buffer,int width,int height) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        printf("[Png]: SaveARGB -> Error fopen: %s\n",filename);
        return -1;
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        printf("[Png]: SaveARGB -> Error write: %s\n",filename);
        fclose(fp);
        return -1;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        printf("[Png]: SaveARGB -> Error info_struct: %s\n",filename);
        png_destroy_write_struct(&png, NULL);
        fclose(fp);
        return -1;
    }

    if (setjmp(png_jmpbuf(png))) {
        printf("[Png]: SaveARGB -> Error jmpbuf: %s\n",filename);
        png_destroy_write_struct(&png, &info);
        fclose(fp);
        return -1;
    }

    png_set_compression_level(png,9);
    png_init_io(png, fp);
    
    png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGBA,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_write_info(png, info);

    png_bytep *rows = (png_bytep *)malloc(sizeof(png_bytep) * height);
    for (int y = 0; y < height; y++) {
        rows[y] = (png_bytep)&buffer[y * width];
    }

    png_write_image(png, rows);
    png_write_end(png, NULL);

    free(rows);
    png_destroy_write_struct(&png, &info);
    fclose(fp);
    return 0;
}
unsigned int* Png_toARGB(const unsigned char* png_data, size_t png_size, int* width, int* height) {
    png_image image;
    memset(&image, 0, sizeof(image));
    image.version = PNG_IMAGE_VERSION;

    if (!png_image_begin_read_from_memory(&image, png_data, png_size)) {
        fprintf(stderr,"[Png]: toArgb -> Error reading: %s\n",image.message);
        return NULL;
    }

    image.format = PNG_FORMAT_RGBA;
    *width = image.width;
    *height = image.height;

    size_t buffer_size = PNG_IMAGE_SIZE(image);
    unsigned char* rgba_buffer = malloc(buffer_size);
    if (!rgba_buffer) {
        printf("[Png]: toArgb -> Error malloc: %s\n",image.message);
        png_image_free(&image);
        return NULL;
    }

    if (!png_image_finish_read(&image, NULL, rgba_buffer, 0, NULL)) {
        fprintf(stderr,"[Png]: toArgb -> Error decoding: %s\n",image.message);
        free(rgba_buffer);
        png_image_free(&image);
        return NULL;
    }

    unsigned int* argb_buffer = malloc(image.width * image.height * sizeof(unsigned int));
    if (!argb_buffer) {
        free(rgba_buffer);
        png_image_free(&image);
        return NULL;
    }

    for (int i = 0; i < image.width * image.height; i++) {
        unsigned char r = rgba_buffer[i * 4 + 0];
        unsigned char g = rgba_buffer[i * 4 + 1];
        unsigned char b = rgba_buffer[i * 4 + 2];
        unsigned char a = rgba_buffer[i * 4 + 3];
        argb_buffer[i] = (a << 24) | (r << 16) | (g << 8) | b;
    }

    free(rgba_buffer);
    png_image_free(&image);
    return argb_buffer;
}
unsigned int* Png_LoadToARGB(const unsigned char* png_data, size_t png_size, int* width, int* height) {
    if (!png_data || png_size < 8) {
        printf("[Png]: LoadToARGB -> Invalid input data.\n");
        return NULL;
    }

    // PNG-Signatur prüfen
    if (png_sig_cmp((png_bytep)png_data, 0, 8)) {
        printf("[Png]: LoadToARGB -> Not a valid PNG buffer.\n");
        return NULL;
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) return NULL;

    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_read_struct(&png, NULL, NULL);
        return NULL;
    }

    if (setjmp(png_jmpbuf(png))) {
        printf("[Png]: LoadToARGB -> Error during decode.\n");
        png_destroy_read_struct(&png, &info, NULL);
        return NULL;
    }

    // Speicher-Leser einrichten
    PngMemReader reader = { png_data, png_size, 0 };
    png_set_read_fn(png, &reader, Png_Read);

    png_read_info(png, info);

    *width  = png_get_image_width(png, info);
    *height = png_get_image_height(png, info);
    png_byte color_type = png_get_color_type(png, info);
    png_byte bit_depth  = png_get_bit_depth(png, info);

    // Normalisierung (immer RGBA, 8 bit)
    if (bit_depth == 16)
        png_set_strip_16(png);
    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png);
    if (png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);
    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    int w = *width;
    int h = *height;

    png_bytep rgba_data = malloc(w * h * 4);
    if (!rgba_data) {
        png_destroy_read_struct(&png, &info, NULL);
        return NULL;
    }

    png_bytep* rows = malloc(sizeof(png_bytep) * h);
    if (!rows) {
        free(rgba_data);
        png_destroy_read_struct(&png, &info, NULL);
        return NULL;
    }

    for (int y = 0; y < h; y++) {
        rows[y] = rgba_data + y * w * 4;
    }

    png_read_image(png, rows);
    png_destroy_read_struct(&png, &info, NULL);
    free(rows);

    // RGBA → ARGB
    unsigned int* argb_buffer = malloc(w * h * sizeof(unsigned int));
    if (!argb_buffer) {
        free(rgba_data);
        return NULL;
    }

    for (int i = 0; i < w * h; i++) {
        unsigned char r = rgba_data[i * 4 + 0];
        unsigned char g = rgba_data[i * 4 + 1];
        unsigned char b = rgba_data[i * 4 + 2];
        unsigned char a = rgba_data[i * 4 + 3];
        argb_buffer[i] = (a << 24) | (r << 16) | (g << 8) | b;
    }

    free(rgba_data);
    return argb_buffer;
}
unsigned int* Png_LoadToARGB_F(const char* filename, int* width, int* height) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        printf("[Png]: LoadToARGB_F -> Error fopen: %s\n", filename);
        return NULL;
    }

    png_byte header[8];
    if (fread(header, 1, 8, fp) != 8 || png_sig_cmp(header, 0, 8)) {
        printf("[Png]: LoadToARGB_F -> Not a valid PNG: %s\n", filename);
        fclose(fp);
        return NULL;
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        fclose(fp);
        return NULL;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_read_struct(&png, NULL, NULL);
        fclose(fp);
        return NULL;
    }

    if (setjmp(png_jmpbuf(png))) {
        printf("[Png]: LoadToARGB_F -> Error reading: %s\n", filename);
        png_destroy_read_struct(&png, &info, NULL);
        fclose(fp);
        return NULL;
    }

    png_init_io(png, fp);
    png_set_sig_bytes(png, 8);
    png_read_info(png, info);

    *width  = png_get_image_width(png, info);
    *height = png_get_image_height(png, info);
    png_byte color_type = png_get_color_type(png, info);
    png_byte bit_depth  = png_get_bit_depth(png, info);

    if (bit_depth == 16)
        png_set_strip_16(png);
    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png);
    if (png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);
    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    int w = *width;
    int h = *height;

    png_bytep* row_pointers = malloc(sizeof(png_bytep) * h);
    png_bytep rgba_data = malloc(w * h * 4);
    if (!row_pointers || !rgba_data) {
        printf("[Png]: LoadToARGB_F -> Error malloc: %s\n", filename);
        free(row_pointers);
        free(rgba_data);
        png_destroy_read_struct(&png, &info, NULL);
        fclose(fp);
        return NULL;
    }

    for (int y = 0; y < h; y++) {
        row_pointers[y] = rgba_data + (y * w * 4);
    }

    png_read_image(png, row_pointers);
    png_destroy_read_struct(&png, &info, NULL);
    fclose(fp);
    free(row_pointers);

    unsigned int* argb_buffer = malloc(w * h * sizeof(unsigned int));
    if (!argb_buffer) {
        free(rgba_data);
        return NULL;
    }

    for (int i = 0; i < w * h; i++) {
        unsigned char r = rgba_data[i * 4 + 0];
        unsigned char g = rgba_data[i * 4 + 1];
        unsigned char b = rgba_data[i * 4 + 2];
        unsigned char a = rgba_data[i * 4 + 3];
        argb_buffer[i] = (a << 24) | (r << 16) | (g << 8) | b;
    }

    free(rgba_data);
    return argb_buffer;
}

int Jpeg_SaveARGB(const char* filename,const unsigned int* argb_buffer,int width,int height,int quality){
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    FILE* outfile = fopen(filename, "wb");
    if (!outfile) {
        printf("[Jpeg]: SaveARGB -> Error fopen: %s\n",filename);
        return 0;
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, outfile);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;// RGB
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo,quality,TRUE); // quality 0–100
    jpeg_start_compress(&cinfo, TRUE);

    JSAMPROW row_pointer;
    unsigned char* row = malloc(width * 3);

    while (cinfo.next_scanline < cinfo.image_height) {
        for (int x = 0; x < width; x++) {
            unsigned int pixel = argb_buffer[cinfo.next_scanline * width + x];
            unsigned char r = (pixel >> 16) & 0xFF;
            unsigned char g = (pixel >> 8) & 0xFF;
            unsigned char b = pixel & 0xFF;

            row[x * 3 + 0] = r;
            row[x * 3 + 1] = g;
            row[x * 3 + 2] = b;
        }

        row_pointer = row;
        jpeg_write_scanlines(&cinfo, &row_pointer, 1);
    }

    free(row);
    jpeg_finish_compress(&cinfo);
    fclose(outfile);
    jpeg_destroy_compress(&cinfo);

    return 1;
}
unsigned char* Jpeg_ByARGB(const unsigned int* argb_buffer,int width,int height,int quality,unsigned long* jpeg_size){
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    unsigned char* jpeg_buf = NULL;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_mem_dest(&cinfo, &jpeg_buf, jpeg_size);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    JSAMPROW row_pointer;
    unsigned char* row = malloc(width * 3);

    while (cinfo.next_scanline < cinfo.image_height) {
        for (int x = 0; x < width; x++) {
            unsigned int pixel = argb_buffer[cinfo.next_scanline * width + x];
            row[x * 3 + 0] = (pixel >> 16) & 0xFF; // R
            row[x * 3 + 1] = (pixel >> 8) & 0xFF;  // G
            row[x * 3 + 2] = pixel & 0xFF;         // B
        }

        row_pointer = row;
        jpeg_write_scanlines(&cinfo, &row_pointer, 1);
    }

    free(row);
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    return jpeg_buf; // Muss mit free() freigegeben werden
}
unsigned int* Jpeg_LoadToARGB(unsigned char* jpeg_data,size_t jpeg_size,int* width,int* height) {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPARRAY buffer;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, jpeg_data, jpeg_size);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    *width = cinfo.output_width;
    *height = cinfo.output_height;
    int row_stride = cinfo.output_width * cinfo.output_components;

    unsigned int* argb_buffer = malloc(cinfo.output_width * cinfo.output_height * sizeof(unsigned int));
    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    for (int y = 0; y < cinfo.output_height; y++) {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        for (int x = 0; x < cinfo.output_width; x++) {
            unsigned char r = buffer[0][x * 3 + 0];
            unsigned char g = buffer[0][x * 3 + 1];
            unsigned char b = buffer[0][x * 3 + 2];
            argb_buffer[y * cinfo.output_width + x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
        }
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    return argb_buffer;
}
unsigned int* Jpeg_LoadToARGB_F(const char* filename,int* width,int* height) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        perror("Fehler beim Öffnen der JPEG-Datei");
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    if (size <= 0) {
        fclose(f);
        fprintf(stderr, "Fehler: Datei leer oder ungültig\n");
        return NULL;
    }

    unsigned char* data = malloc(size);
    if (!data) {
        fclose(f);
        fprintf(stderr, "Fehler: Speicher konnte nicht alloziert werden\n");
        return NULL;
    }

    size_t readsize = fread(data, 1, size, f);
    while(readsize <= 0) readsize = fread(data, 1, size, f);
    fclose(f);

    unsigned int* result = Jpeg_LoadToARGB(data, size, width, height);
    free(data);

    return result;
}
void ARGB_toYUYV(unsigned int* buffer,int width,int height,unsigned char* out,int length) {
    int yuyv_index = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x += 2) {
            unsigned int pixel1 = buffer[y * width + x];
            unsigned int pixel2 = buffer[y * width + (x + 1)];

            //unsigned char a1 = (pixel1 >> 24) & 0xFF;
            unsigned char r1 = (pixel1 >> 16) & 0xFF;
            unsigned char g1 = (pixel1 >> 8) & 0xFF;
            unsigned char b1 = pixel1 & 0xFF;

            //unsigned char a2 = (pixel2 >> 24) & 0xFF;
            unsigned char r2 = (pixel2 >> 16) & 0xFF;
            unsigned char g2 = (pixel2 >> 8) & 0xFF;
            unsigned char b2 = pixel2 & 0xFF;

            // Berechnung von YUYV Werten für zwei Pixel
            unsigned char y1 = (unsigned char)(0.299 * r1 + 0.587 * g1 + 0.114 * b1);
            unsigned char u1 = (unsigned char)(-0.14713 * r1 - 0.28886 * g1 + 0.436 * b1);
            //unsigned char v1 = (unsigned char)(0.615 * r1 - 0.51499 * g1 - 0.10001 * b1);

            unsigned char y2 = (unsigned char)(0.299 * r2 + 0.587 * g2 + 0.114 * b2);
            //unsigned char u2 = (unsigned char)(-0.14713 * r2 - 0.28886 * g2 + 0.436 * b2);
            unsigned char v2 = (unsigned char)(0.615 * r2 - 0.51499 * g2 - 0.10001 * b2);

            // YUYV: Y1 U Y2 V
            out[yuyv_index++] = y1;
            out[yuyv_index++] = u1;
            out[yuyv_index++] = y2;
            out[yuyv_index++] = v2;
        }
    }

    // for (int i = 0; i < width * height; i += 2) {
    //     unsigned char a0 = buffer[i * 4 + 0]; // A
    //     unsigned char r0 = buffer[i * 4 + 1];
    //     unsigned char g0 = buffer[i * 4 + 2];
    //     unsigned char b0 = buffer[i * 4 + 3];
    //     // Pixel 2
    //     unsigned char a1 = buffer[(i + 1) * 4 + 0];
    //     unsigned char r1 = buffer[(i + 1) * 4 + 1];
    //     unsigned char g1 = buffer[(i + 1) * 4 + 2];
    //     unsigned char b1 = buffer[(i + 1) * 4 + 3];
    //     // YUV berechnen
    //     unsigned char y0 = (unsigned char)(0.299 * r0 + 0.587 * g0 + 0.114 * b0);
    //     unsigned char y1 = (unsigned char)(0.299 * r1 + 0.587 * g1 + 0.114 * b1);
    //     unsigned char u  = (unsigned char)(((-0.169 * r0 - 0.331 * g0 + 0.5 * b0) +
    //                             (-0.169 * r1 - 0.331 * g1 + 0.5 * b1)) / 2 + 128);
    //     unsigned char v  = (unsigned char)(((0.5 * r0 - 0.419 * g0 - 0.081 * b0) +
    //                             (0.5 * r1 - 0.419 * g1 - 0.081 * b1)) / 2 + 128);
    //     // In YUYV speichern
    //     out[i * 2 + 0] = y0;
    //     out[i * 2 + 1] = u;
    //     out[i * 2 + 2] = y1;
    //     out[i * 2 + 3] = v;
    // }
}

#endif

#if defined IMAGE_PNGJPEG

#define IMAGE_FLIP_NONE     0
#define IMAGE_FLIP_V        1
#define IMAGE_FLIP_H        2

char Image_FlipState = 0;

void Image_Enable_FlipV(){
    Image_FlipState |= IMAGE_FLIP_V;
}
void Image_Disable_FlipV(){
    Image_FlipState &= ~IMAGE_FLIP_V;
}
void Image_Enable_FlipH(){
    Image_FlipState |= IMAGE_FLIP_H;
}
void Image_Disable_FlipH(){
    Image_FlipState &= ~IMAGE_FLIP_H;
}

int Image_Save(char* filename,unsigned int* buffer,int width,int height) {
    CStr type = Files_Type(filename);
    
    if(CStr_Cmp(type,".png") || CStr_Cmp(type,".PNG")){
        Png_SaveARGB(filename,buffer,width,height);
    }else if(CStr_Cmp(type,".jpg") || CStr_Cmp(type,".jpeg") || CStr_Cmp(type,".JPG") || CStr_Cmp(type,".JPEG")){
        Jpeg_SaveARGB(filename,buffer,width,height,85);
    }else{
        printf("[Image]: Save -> Error format not valid: %s (%s)\n",type,filename);
    }

    CStr_Free(&type);
    return 0;
}
unsigned int* Image_Load(char* filename,int* width,int* height) {
    CStr type = Files_Type(filename);
    
    if(CStr_Cmp(type,".png") || CStr_Cmp(type,".PNG")){
        CStr_Free(&type);
        unsigned int* buffer = Png_LoadToARGB_F(filename,width,height);
        
        if(Image_FlipState & IMAGE_FLIP_V){
            for(int i = 0;i<*height;i++){
                for(int j = 0;j<*width;j++){
                    const int swap_j = (*width - 1) - j;
                    buffer[i * *width + j] = buffer[i * *width + swap_j];
                }
            }
        }
        if(Image_FlipState & IMAGE_FLIP_H){
            for(int i = 0;i<*height;i++){
                const int swap_i = (*height - 1) - i;
                for(int j = 0;j<*width;j++){
                    buffer[i * *width + j] = buffer[swap_i * *width + j];
                }
            }
        }

        return buffer;
    }else if(CStr_Cmp(type,".jpg") || CStr_Cmp(type,".jpeg") || CStr_Cmp(type,".JPG") || CStr_Cmp(type,".JPEG")){
        CStr_Free(&type);
        unsigned int* buffer = Jpeg_LoadToARGB_F(filename,width,height);

        if(Image_FlipState & IMAGE_FLIP_V){
            for(int i = 0;i<*height;i++){
                for(int j = 0;j<*width;j++){
                    const int swap_j = (*width - 1) - j;
                    buffer[i * *width + j] = buffer[i * *width + swap_j];
                }
            }
        }
        if(Image_FlipState & IMAGE_FLIP_H){
            for(int i = 0;i<*height;i++){
                const int swap_i = (*height - 1) - i;
                for(int j = 0;j<*width;j++){
                    buffer[i * *width + j] = buffer[swap_i * *width + j];
                }
            }
        }

        return buffer;
    }else{
        printf("[Image]: Load -> Error format not valid: %s (%s)\n",type,filename);
    }

    CStr_Free(&type);
    return NULL;
}

#else

#define STB_IMAGE_IMPLEMENTATION
#include "Stb_Image.h"

//#define STB_IMAGE_IMPLEMENTATION
//#include "Stb_Image_Write.h"

void Image_Enable_FlipV(){
    stbi_set_flip_vertically_on_load(1);
}
void Image_Disable_FlipV(){
    stbi_set_flip_vertically_on_load(0);
}

int Image_Save(const char *filename,unsigned int* buffer,int width,int height) {
    // Save as PNG
    //int channels = 4;
    //if(!stbi_write_png(filename,width,height,channels,buffer,width * channels)) {
    //    printf("[Image]: Save -> Failed to save image.\n");
    //}
    return -1;
}
unsigned int* Image_Load(const char* filename,int* width,int* height) {
    int channels;
    unsigned char* data = stbi_load(filename,width,height,&channels,4);
    return (unsigned int*)data;
}

#endif

#endif // !IMAGE_H