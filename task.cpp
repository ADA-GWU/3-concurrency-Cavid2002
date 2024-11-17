#include <thread>
#include <iostream>
#include "Image.h"


Pixel find_mean_pixel(Image* img, int x1, int y1, int x2, int y2)
{   
    uint32_t c = 0, m = 0, index;
    uint32_t rgb[3];
    Pixel res;

    for(int y = y1; y < y2; y++)
    {
        for(int x = x1; x < x2; x++)
        {
            for(int ch = 0; ch < 3; ch++)
            {
                index = (img->height * img->width + x) * img->chanels + ch;
                rgb[ch] += img->arr[index];
            }
            c++;
        }
    }

    res.r = rgb[0] / c;
    res.g = rgb[1] / c;
    res.b = rgb[2] / c;

    return res; 
    
}


void apply_pixel(Image* img, int x1, int y1, int x2, int y2)
{
    
}


void foo(Image* img, uint32_t id, uint32_t core_count, uint32_t sq_size)
{

    
    for(int y = id * sq_size; y < img->height; y += sq_size)
    {
        for(int x = id * sq_size; x < img->width; x += sq_size)
        {

            

        }
    }
    
} 


int main()
{
    
    int core_count = std::thread::hardware_concurrency();
    
    std::thread th[core_count];
    
    int sqare_size = 50;
    

    for(int i = 0; i < core_count; i++)
    {
        th[i] = std::thread()
    }


    
}
