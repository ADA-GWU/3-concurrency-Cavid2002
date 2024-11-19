#include <thread>
#include <iostream>
#include <chrono>
#include "Image.h"


Pixel find_mean_pixel(Image* img, int x1, int y1, int x2, int y2)
{   
    uint32_t c = 0, m = 0, index;
    uint32_t chanels[3] = {0, 0, 0};
    Pixel res;

    if(x2 > img->width) x2 = img->width;
    if(y2 > img->height) y2 = img->height;

    for(int y = y1; y < y2; y++)
    {
        for(int x = x1; x < x2; x++)
        {
            for(int ch = 0; ch < 3; ch++)
            {
                index = (y * img->width + x) * img->chanels + ch;
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
    if(x2 > img->width) x2 = img->width;
    if(y2 > img->height) y2 = img->height;


    for(int y = y1; y < y2; y++)
    {
        for(int x = x1; x < x2; x++)
        {
            index = (y * img->width + x) * img->chanels;

            img->arr[index] = p.r;
            img->arr[index + 1] = p.g;
            img->arr[index + 2] = p.b;
            
        }
    }

}


void process_img(Image* img, uint32_t id, uint32_t core_count, uint32_t sq_size)
{
    Pixel mean;
    int increment = core_count * sq_size;
    for(int y = id * sq_size; y < img->height; y += increment)
    {
        for(int x = 0; x < img->width; x += sq_size)
        {
            mean = find_mean_pixel(img, x, y, x + sq_size, y + sq_size);
            apply_pixel(img, mean, x, y, x + sq_size, y + sq_size);
        }   
    }

} 



void multithreaded_mode(Image* img, int sq_size)
{
    int core_count = std::thread::hardware_concurrency();
    
    std::cout << "Number of created threads: " << core_count << std::endl;
    std::thread* th = new std::thread[core_count];
    
    for(int i = 0; i < core_count; i++)
    {
        th[i] = std::thread(process_img, img, i, core_count, sq_size);
    }
    
    for(int i = 0; i < core_count; i++)
    {
        th[i].join();
    }
    
    delete[] th;
}



int main(int argc, char** argv)
{   
    
    if(argc != 4)
    {
        fatal_error("Usage: [filename] [square_size] [processing_mode]");
    }

    Image img = load_image(argv[1]);
    int sq_size = atoi(argv[2]);

    double result;
    if(argv[3][0] == 'S')
    {
        std::cout << "Single threaded mode selected..." << std::endl;
        auto start = std::chrono::steady_clock::now();
        process_img(&img, 0, 1, sq_size);
        auto finish = std::chrono::steady_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);

        std::cout << "Elapsed time in millsec: " << duration.count() << std::endl;

    }
    else if(argv[3][0] == 'M')
    {

        auto start = std::chrono::steady_clock::now();
        multithreaded_mode(&img, sq_size);
        auto finish = std::chrono::steady_clock::now();

        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);

        std::cout << "Elapsed time in millsec: " << duration.count() << std::endl;
    }
    else
    {
        fatal_error("Invalid processing mode! Choose from: [M] or [S]");
    }
    
    std::cout << "Writing back result..." << std::endl;
    write_image("result.jpg", img);
    free_image(img);

    return 0;
    
}
