# Project Overview:

When a dynamic memory is allocated on the heap in C++, it is not freed automatically till the end of the program. I built a wrapper around the heap memory allocator and de-allocator functions like malloc and free to check for and avoid memory leaks in a program. This project helped me learn about memory management and garbage collection so that I can write highly optimized and efficient code. 

# Implementation Details:

I implemented a wrapper around the following functions:\
✔️ malloc\
✔️ calloc\
✔️ realloc\
✔️ free

I used a character at the beginning of my metadata to contain information about the state of the pointer (i.e., freed or not). I also used a character to demarcate the end of the allocated block. I padded the total allocated memory with an extra (total%8) bytes of memory so that the entire block would allign with double.


# Tools Used
* C++
* Memory management
* Garbage collection
* Smart-pointers

# Contributing
#### Step 1

- **Option 1**
    - 🍴 Fork this repo!

- **Option 2**
    - 👯 Clone this repo to your local machine.


#### Step 2

- **Build your code**

#### Step 3

- 🔃 Create a new pull request.