#include <iostream>
#include <string>
#include <unistd.h>
#include <pthread.h>
// #include <stdio.h>

#define NTHREADS 5
#define COUNT 5000

void *inc_x(void *x_void_ptr)
{
    int *x_ptr = (int *)x_void_ptr;

    for (int i = 0; i < COUNT; i++)
    {
        (*x_ptr)++;
    }

    return NULL;
}

int main()
{

    static int x = 0; // static stays same location being shared by all threads
    pthread_t tid[NTHREADS];
    std::cout << "Initial value of x: " << x << std::endl;

    for (int i = 0; i < NTHREADS; i++)
    {
        if (pthread_create(&tid[i], NULL, inc_x, &x)) // (address where we store tid, location of default stack ptr if null, name of special function, ptr to void that represents args we're passing)
        { 
            std::cout << "Error creating thread" << std::endl;
            return 1;
        }
        // pthread_join(tid[i], nullptr);
    }

    for(int i = 0; i < NTHREADS; i++) {
        pthread_join(tid[i], NULL);
    }

    std::cout << "Final value of x: " << x << std::endl;

    return 0;
}