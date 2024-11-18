#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include "./include/stb_image.h"
#include "./include/stb_image_write.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>


typedef struct{
    int32_t width;
    int32_t height;
    int32_t chanels;
    uint8_t* arr;
} Image;

typedef struct{
    uint8_t r, g, b;
} Pixel;


void fatal_error(const char* msg)
{
    fprintf(stderr, "[ERROR]%s\n", msg);
    exit(EXIT_FAILURE);
}

Image load_image(const char* filename)
{
    Image img;
    img.arr = stbi_load(filename, &img.width, &img.height, &img.chanels, 0);
    if(img.arr == NULL)
    {
        fatal_error("Image load failure!");
    }
    return img;
}

void write_image(const char* filename, Image img)
{
    if(strstr(filename, ".png") != NULL || strstr(filename, ".PNG") != NULL)
    {
        stbi_write_png(filename, img.width, img.height, img.chanels, img.arr, img.width * img.chanels);
        return;
    }

    if(strstr(filename, ".jpg") != NULL || strstr(filename, ".jpeg") != NULL)
    {
        stbi_write_jpg(filename, img.width, img.height, img.chanels, img.arr, 100);
        return;
    }

    fatal_error("Aviable Formats are .jpg or .png");
}


void free_image(Image img)
{
    stbi_image_free(img.arr);
}
