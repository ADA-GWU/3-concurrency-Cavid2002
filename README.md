
# Image Processing with multithreading #

## Introduction: ##

The following report is written to provide explanation to the task that requires to program image quantisation software. The task requires to program this task using both single threading and multi-threading then compare the results.

To implement this program the language of choice was C++, even though the the author of this report prefers the C over any other programming language. The main reason of using C++ instead of C is the fact that the process of creating threads in highly OS-dependent and C-standart library doesn't provide any OS-independent threading interface, as the result there were only 2 options:

1. Write OS-independent interface from the scratch which involves going through the OS API and use conditional compilation which is too time-consuming and ugly. Consider example from below:

```C
#if defined(_WIN32)
#include <windows.h> 
#elif defined(__linux__)
#include <pthread.h>
#endif

void creating_thread()
{
#if defined(_WIN32)
    // Windows-specific code
#elif defined(__APPLE__)
    // macOS-specific code
#elif defined(__linux__)
    // linux-specific code
#else
    // Other platforms
#endif

}

```

2. Use C++ that provides OS-independent threading library called `std::thread`. Which is less complicated, however in the background this library is still just a combination of conditional compilation with native OS API-s same as in example provided above.


```C++
#include <thread>


void func(int args)
{

}


int main()
{
    std::thread th(func, args);
    th.detach();
}

```

The second method has been chosen for implementation. It's important to note that this task involves pure parallel processing for image handling, so there's no need to use synchronization mechanisms such as mutex locks or semaphores.

For handling image files for this I decided to use good old [stb_image library](https://github.com/nothings/stb). As mentioned in the previous reports this is single header libraries and not additional precompiled libraries are needed at link-time. *stb_image* headers are located inside *./include* directory. There is also additional `"Image.h"` header written by the author of this report which contains data structure to represent the image and picture elements alongside with several function to make interraction with *stb_image* library much easier.


```C
typedef struct{
    int32_t width;
    int32_t height;
    int32_t chanels;
    uint8_t* arr;
} Image;

typedef struct{
    uint8_t r, g, b;
} Pixel;

Image load_image(const char* filename);
void write_image(const char* filename, Image img);
void free_image(Image img);
```


## Explaining Multi-Threading ##

As we already know thread is an OS entity that recides in the same address space and allows to take advantage of parallel computing. Threads make it possible for a program to devide the data into independent parts and feed them to multi-core CPU to achieve significant perfomance boost. An example of the data that can be devided and proccessed independently are graphics files no wonder why multi-threading is used in computer vision and game development.

Even though address space is shared each thread has its own stack space provided by OS. Which actually makes sense considering that threads are usually bound to functions which require to initialize stack frame for local variables.  

But does that mean that single core CPU cannot utilize multithreading?

Single core CPU's can still use multithreading, however in that case scheduler will just switch between them. As the result even though single core can utulize multithreading there would little to no perfomance gain.


## Multi-threaded Processing ##

We can treat the input image as grid in which each element is `NxN` square (N is size of square provided as the input). Then via special function provided by C++ standart threading library `std::thread::hardware_concurrency()` we can get the number of logical cores on cpu which at the same time is maximum number of threads we can create without overwhelming CPU. Then the array of the thread object is created that contains the pointer to the function the thread should execute(*process_img* in this case), state of the thread and pointers to `join()` and `detach()` functions. 

```C++
int core_count = std::thread::hardware_concurrency();
    
std::thread* th = new std::thread[core_count];

for(int i = 0; i < core_count; i++)
{
    th[i] = std::thread(proccess_img, &img, i, core_count, sq_size);
}
```

The signature of function `process_img` looks like as follows:

```C
void process_img(Image* img, uint32_t id, uint32_t core_count, uint32_t sq_size)
{
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
```

The function accepts image object, id of the thread executing function, thread count and box size. Thread count is used to calculate the increment along Y and thread id needed to define starting point for the loop. Reading the code carefully it is easy to come into conclusion that each thread will process separate row and apply pixel average. The diagram below demonstrates how it will work if the application runs on 4 core CPU. 

```
            +---+---+---+---+---+---+---+---+
thread 0 -> |   |   |   |   |   |   |   |   |
            +---+---+---+---+---+---+---+---+
thread 1 -> |   |   |   |   |   |   |   |   |
            +---+---+---+---+---+---+---+---+
thread 2 -> |   |   |   |   |   |   |   |   |
            +---+---+---+---+---+---+---+---+
thread 3 -> |   |   |   |   |   |   |   |   |
            +---+---+---+---+---+---+---+---+
thread 0 -> |   |   |   |   |   |   |   |   |
            +---+---+---+---+---+---+---+---+
thread 1 -> |   |   |   |   |   |   |   |   |
            +---+---+---+---+---+---+---+---+
thread 2 -> |   |   |   |   |   |   |   |   |
            +---+---+---+---+---+---+---+---+
thread 3 -> |   |   |   |   |   |   |   |   |
            +---+---+---+---+---+---+---+---+

```

