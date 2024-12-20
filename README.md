
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

2. Use C++ that provides OS-independent threading library called `std::thread`, which is less complicated, however in the background this library is still just a combination of conditional compilation with native OS API-s same as in example provided above. The usage of the std::thread library is quite straightforward.


```C++
#include <thread>


void func(int args)
{

}


int main()
{
    std::thread th(func, args);
    th.join(); // wait thread to complete
}

```

The second method has been chosen for this implementation. It's important to note that this task involves pure parallel processing for image handling, so there's no need to use synchronization mechanisms such as mutex locks or semaphores.

For handling image files it was decided to use good old [stb_image library](https://github.com/nothings/stb). As mentioned in the previous report this library provides simple low-level interface for accessing and manipulating image files. As being single header library, it  requires no additional precompiled libraries at link-time. *stb_image* headers are located inside *./include* directory. There is also additional `"Image.h"` header written by the author of this report which contains data structures to represent the image and picture elements alongside with several function to make interraction with *stb_image* library much more easier.

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


The big and robust libraries like **OPENCV** for handling images is not going to be inefficent as creating suitable environment for it, is quite complicated and as the result would harm the entire portability of the program in general. **STB_IMAGE** is preferable option in this case because as being simple low-level OS-independent library it pushes developer to write algorithms from the scratch without any harm to portability. 


## Explaining Multi-Threading ##

As we already know thread is an OS entity that recides in the same address space and allows to take advantage of parallel computing. Threads make it possible for a program to divide the data into independent parts and feed them to multi-core CPU to achieve significant perfomance boost. An example of the data that can be devided and processed independently are graphics files no wonder why multi-threading is used in computer vision and game development.

Even though address space is shared each thread has its own stack space provided by OS. Which actually makes sense considering that threads are usually bound to functions which require to initialize stack frame for local variables.  

But does that mean that single core CPU cannot utilize multithreading?

Single core CPU's can still use multithreading, however in that case scheduler will just switch between them. As the result even though single core can utulize multithreading there would little to no perfomance gain.

Another thing to mention is that the when the process wants to create a thread it should make special system call to OS. As I mentioned in the previous reports system calls involves the contex switch which is extremly expensive in terms of perfomance. So for short paralel computational purposes creation of threads must be avoided.


## Multi-threaded Processing ##

We can treat the input image as grid in which each element is `NxN` square (N is size of square provided as the input). Then via special function provided by C++ standart threading library `std::thread::hardware_concurrency()` we can get the number of logical cores on cpu which at the same time is maximum number of threads we can create without overwhelming CPU. Then the array of the thread objects is created. Thread object contains the pointer to the function the thread should execute(*process_img* in this case), state of the thread and pointers to `join()` and `detach()` functions. 

```C++
int core_count = std::thread::hardware_concurrency();
    
std::thread* th = new std::thread[core_count];

for(int i = 0; i < core_count; i++)
{
    th[i] = std::thread(proccess_img, &img, i, core_count, sq_size);
}
```

The function accepts image object, id of the thread executing function, thread count and box size. Thread count is used to calculate the increment along Y and thread id needed to define starting point for the loop. The structure of function `process_img` looks like as follows:

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

Reading the code carefully it is easy to come into conclusion that each thread will process separate row and apply pixel average. The diagram below demonstrates how it will work if the application would run on 4 core CPU. 

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

Each thread will go along the row column by column and will find the mean pixel for each channel (R G B) and overwrite entire grid box pixels with values stored inside mean object. The algorithm for calculating and applying mean values to the grid element is quite straightforward so this part will be skipped and we will proceed with results.

## Image Results and Duration Measurement: ##

Before proceeding with results first the compilation process should be demonstrated. It is command is same for both Windows and Unix-like OS's.

```Bash
g++ task.cpp -o task
```

The generated executable should be provided the following parameters:

```Bash
./task [filename] [square_size] [processing_mode]
```

The images with the input parameters and results are provided below:

`Sample Image:`
![img](./img/coast.jpg)

----
### Command-1: ### 
For Linux or Mac: `./task img/coast.jpg 50 S` or `./task img/coast.jpg 50 M`

For Windows: ` .\task.exe .\img\coast.jpg 50 S` or ` .\task.exe .\img\coast.jpg 50 M`

`Result-1:`

![img](./img/result1.jpg)

`Output-1 (Single-threaded):`
```
Single threaded mode selected...
Elapsed time: 0.071
```
`Output-1 (Multi-threaded):`
```
Multi-threaded mode selected...
Number of threads: 12
Elapsed time in milliseconds: 0.016
```

----
### Command-2: ### 

For Linux or Mac: `./task img/coast.jpg 100 S` or `./task img/coast.jpg 100 M`

For Windows: ` .\task.exe .\img\coast.jpg 100 S` or ` .\task.exe .\img\coast.jpg 100 M`


`Output-2 (Single-threaded):`
```
Single threaded mode selected...
Elapsed time: 0.068
```
`Output-2 (Multi-threaded):`
```
Multi-threaded mode selected...
Number of threads: 12
Elapsed time in milliseconds: 0.018
```

`Result-2:`

![img](./img/result2.jpg)

----
### Command-3: ### 

For Linux or Mac: `./task img/coast.jpg 200 S` or `./task img/coast.jpg 200 M`

For Windows: ` .\task.exe .\img\coast.jpg 200 S` or ` .\task.exe .\img\coast.jpg 200 M`


`Result-3:`

![img](./img/result3.jpg)


`Output-3 (Single-threaded):`
```
Single threaded mode selected...
Elapsed time: 0.062
```
`Output-3 (Multi-threaded):`
```
Multi-threaded mode selected...
Number of threads: 12
Elapsed time in milliseconds: 0.017
```

By viewing the output images above we can conclude that program works properly as expected. In terms of execution durations, even with the performance hog in the form of system calls and context switches multithreaded mode would run 3-4 times faster than single-threaded one. The performance gap will increase as size of input image grows. 

However as the size of the grid box increases the gap between multithreading and singlethreading decreases. The reason for as mentioned before is that as square box size grows the operations required to perform by each thread decreases as the the result while single threaded program will start processing immedietly the multithreaded one would make system calls for creating the threads wasting a lot of time. 
