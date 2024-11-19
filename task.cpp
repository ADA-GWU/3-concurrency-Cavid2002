#include <thread>
#include <iostream>
#include <ctime>
#include <cstring>

#include "./include/Image.h"



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


void process_img(Image* img, uint32_t id, uint32_t thread_num, uint32_t sq_size)
{
    Pixel mean;
    int increment = thread_num * sq_size;
    for(int y = id * sq_size; y < img->height; y += increment)
    {
        for(int x = 0; x < img->width; x += sq_size)
        {
            mean = find_mean_pixel(img, x, y, x + sq_size, y + sq_size);
            apply_pixel(img, mean, x, y, x + sq_size, y + sq_size);
        }   
    }

} 



void multithreaded_mode(Image* img, int thread_num, int sq_size)
{
    std::thread* th = new std::thread[thread_num];
    
    for(int i = 0; i < thread_num; i++)
    {
        th[i] = std::thread(process_img, img, i, thread_num, sq_size);
    }
    
    for(int i = 0; i < thread_num; i++)
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

    std::clock_t start, stop;
    double result;
    if(argv[3][0] == 'S')
    {
        std::cout << "Single threaded mode selected..." << std::endl;

        start = std::clock();
        process_img(&img, 0, 1, sq_size);
        stop = std::clock();

    }
    else if(argv[3][0] == 'M')
    {  
        int thread_num = std::thread::hardware_concurrency();
        std::cout << "Number of threads: " << thread_num << std::endl;

        start = std::clock();
        multithreaded_mode(&img, thread_num, sq_size);
        stop = std::clock();
    }
    else
    {
        fatal_error("Invalid processing mode! Choose from: [M] or [S]");
    }
    
    result = (double)(stop - start) / CLOCKS_PER_SEC;

    std::cout << "Elapsed time: " << result << std::endl;

    
    write_image("result.jpg", img);
    
    free_image(img);

    return 0;
    
}
