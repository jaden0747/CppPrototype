#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>

struct SharedData {
    int             initialized;   // set to 1 by writer after full init
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

    // Try to unlink any previous failed shared memory state
    if (shm_unlink(name) == 0)
    {
        std::cout << "Writer: Cleared previous shared memory segment." << std::endl;
    }

    // Create shared memory object
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1)
    {
        std::perror("shm_open");
        return 1;
    }

    // Configure the size of the shared memory object
    if (ftruncate(shm_fd, SIZE) == -1)
    {
        std::perror("ftruncate");
        return 1;
    }

    // Memory map the shared memory object
    void* ptr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        std::perror("mmap");
        return 1;
    }

    SharedData* data = static_cast<SharedData*>(ptr);
    data->initialized = 0;  // mark not ready until fully initialized

    // Initialize mutex and condition variables for process-shared use
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&data->mutex, &attr);

    pthread_condattr_t cond_attr;
    pthread_condattr_init(&cond_attr);
    pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&data->data_ready, &cond_attr);
    pthread_cond_init(&data->data_consumed, &cond_attr);
    pthread_condattr_destroy(&cond_attr);
    pthread_mutexattr_destroy(&attr);

    data->has_data    = false;
    data->initialized = 1;  // signal to reader that init is complete

    std::cout << "Writer: started, using condition variables..." << std::flush;
    int i = 0;
    while (true) {
        i++;
        pthread_mutex_lock(&data->mutex);

        // Write new message and set flag
        snprintf(data->message, sizeof(data->message), "Message %d from writer!", i);
        data->has_data = true;
        std::cout << "\nWriter: sent message '" << data->message << "'" << std::flush;

        // Signal reader that data is ready
        pthread_cond_broadcast(&data->data_ready);

        // Wait for reader to consume the data
        std::cout << " (waiting for reader...)" << std::flush;
        while (data->has_data) {
            pthread_cond_wait(&data->data_consumed, &data->mutex);
        }
        std::cout << " (acknowledged)" << std::flush;

        pthread_mutex_unlock(&data->mutex);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Never reached in this loop, but for completeness:
    pthread_cond_destroy(&data->data_ready);
    pthread_cond_destroy(&data->data_consumed);
    pthread_mutex_destroy(&data->mutex);
    munmap(ptr, SIZE);
    close(shm_fd);
    return 0;
}
