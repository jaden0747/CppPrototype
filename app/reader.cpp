#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>

struct SharedData
{
    int             initialized; // set to 1 by writer after full init
    pthread_mutex_t mutex;
    pthread_cond_t  data_ready;    // signals new data available
    pthread_cond_t  data_consumed; // signals data was consumed
    bool            has_data;      // flag to indicate new data
    char            message[256];
};

int main()
{
    const char* name = "/shm_example";
    const int   SIZE = sizeof(SharedData);

    // // Try to unlink any previous failed shared memory state
    // if (shm_unlink(name) == 0) {
    //     std::cout << "Reader: Cleared previous shared memory segment." << std::endl;
    // }

    // Open the shared memory object
    int shm_fd = -1;
    while (true)
    {
        shm_fd = shm_open(name, O_RDWR, 0666);
        if (shm_fd == -1)
        {
            std::perror("shm_open");
            std::cout << "Reader: waiting for writer to create shared memory..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        else
        {
            break;
        }
    }

    // Memory map the shared memory object
    void* ptr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED)
    {
        std::perror("mmap");
        close(shm_fd);
        return 1;
    }

    SharedData* data = static_cast<SharedData*>(ptr);

    // Wait for writer to finish initializing mutex and condvars
    std::cout << "Reader: waiting for writer to initialize..." << std::flush;
    while (!data->initialized)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << " ready!" << std::endl;

    std::cout << "Reader: using condition variables..." << std::flush;
    while (true)
    {
        pthread_mutex_lock(&data->mutex);

        // Wait for writer to signal new data
        while (!data->has_data)
        {
            std::cout << "." << std::flush; // Progress indicator
            pthread_cond_wait(&data->data_ready, &data->mutex);
        }

        std::cout << "\nReader: received message: '" << data->message << "'" << std::flush;

        // Clear flag and signal writer that we've consumed the data
        data->has_data = false;
        pthread_cond_broadcast(&data->data_consumed);
        std::cout << " (acknowledged)" << std::flush;

        pthread_mutex_unlock(&data->mutex);
    }

    // Never reached, but for completeness:
    pthread_cond_destroy(&data->data_ready);
    pthread_cond_destroy(&data->data_consumed);
    munmap(ptr, SIZE);
    close(shm_fd);

    // Optionally remove the shared memory object
    if (shm_unlink(name) == 0)
    {
        std::cout << "Reader: Cleaned up shared memory segment." << std::endl;
    }
    else
    {
        std::perror("shm_unlink");
    }

    return 0;
}
