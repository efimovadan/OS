#include <iostream>
#include <string>
#ifdef _WIN32
#include <windows.h>
#else
#include <semaphore.h>
#include <pthread.h>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#endif


class CrossPlatformMutex {
public:
    explicit CrossPlatformMutex(const std::string& name);
    ~CrossPlatformMutex();

    void lock();
    void unlock();

private:
    std::string name_;

#ifdef _WIN32
    HANDLE hMutex_ = nullptr;
#else
    sem_t* sem_ = nullptr;
    std::string semaphore_name_;
#endif
};
