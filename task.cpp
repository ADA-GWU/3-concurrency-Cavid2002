#include <thread>
#include <iostream>
#include "Image.h"


Pixel find_mean_pixel(Image* img, int x1, int y1, int x2, int y2)
{   
    uint32_t c = 0, m = 0, index;
    uint32_t chanels[4];
    Pixel res;

    if(x2 > img->width) x2 = img->width;
    if(y2 > img->height) y2 = img->height;

    for(int y = y1; y < y2; y++)
    {
        for(int x = x1; x < x2; x++)
        {
            for(int ch = 0; ch < 3; ch++)
            {
                index = (img->height * img->width + x) * img->chanels + ch;
                chanels[ch] += img->arr[index];
            }
            c++;
        }
    }

    res.r = chanels[0] / c;
    res.g = chanels[1] / c;
    res.b = chanels[2] / c;

    return res; 
    
}


void apply_pixel(Image* img, Pixel p, int x1, int y1, int x2, int y2)
{
    uint32_t index;
    for(int y = y1; y < y2; y++)
    {
        for(int x = x1; x < x2; x++)
        {
            index = (img->height * img->width + x) * img->chanels;

            img->arr[index] = p.r;
            img->arr[index + 1] = p.g;
            img->arr[index + 2] = p.b;
            
        }
    }

}


void foo(Image* img, uint32_t id, uint32_t core_count, uint32_t sq_size)
{
    Pixel mean;
    int increment = core_count * sq_size;
    for(int y = id * sq_size; y < img->height; y += increment)
    {
        for(int x = id * sq_size; x < img->width; x += increment)
        {

            mean = find_mean_pixel(img, x, y, x + sq_size, y + sq_size);
            apply_pixel(img, mean, x, y, x + sq_size, y + sq_size);

        }
    }
    
} 


int main()
{
    
    int core_count = std::thread::hardware_concurrency();
    
    std::thread* th = new std::thread[core_count];
    
    int sq_size = 50;
    
    Image img = load_image("image.jpg");


    for(int i = 0; i < core_count; i++)
    {
        th[i] = std::thread(foo, &img, i, core_count, sq_size);
    }


    for(int i = 0; i < core_count; i++)
    {
        th[i].join();
    }

    write_image("result.jpg", img);
    free_image(img);

    delete[] th;
    
}
