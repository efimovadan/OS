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
    explicit CrossPlatformMutex(const std::string& name) : name_(name) {
#ifdef _WIN32
        hMutex_ = CreateMutexA(nullptr, FALSE, ("Global\\" + name_).c_str());
        if (!hMutex_) {
            throw std::runtime_error("Failed to create/open mutex: " + std::to_string(GetLastError()));
        }
#else
        semaphore_name_ = "/" + name_;
        sem_ = sem_open(semaphore_name_.c_str(), O_CREAT, 0666, 1);
        if (sem_ == SEM_FAILED) {
            throw std::runtime_error("Failed to create/open semaphore: " + std::string(strerror(errno)));
        }
#endif
    }

    ~CrossPlatformMutex() {
#ifdef _WIN32
        if (hMutex_) {
            CloseHandle(hMutex_);
        }
#else
        if (sem_ != SEM_FAILED) {
            sem_close(sem_);
            sem_unlink(semaphore_name_.c_str());
        }
#endif
    }

    void lock() {
#ifdef _WIN32
        DWORD waitResult = WaitForSingleObject(hMutex_, INFINITE);
        if (waitResult != WAIT_OBJECT_0) {
            throw std::runtime_error("Failed to lock mutex: " + std::to_string(GetLastError()));
        }
#else
        if (sem_wait(sem_) != 0) { 
            throw std::runtime_error("Failed to lock semaphore: " + std::string(strerror(errno)));
        }
#endif
    }

    void unlock() {
#ifdef _WIN32
        if (!ReleaseMutex(hMutex_)) {
            throw std::runtime_error("Failed to unlock mutex: " + std::to_string(GetLastError()));
        }
#else
        if (sem_post(sem_) != 0) {
            throw std::runtime_error("Failed to unlock semaphore: " + std::string(strerror(errno)));
        }
#endif
    }

private:
    std::string name_;
#ifdef _WIN32
    HANDLE hMutex_ = nullptr;
#else
    sem_t* sem_ = nullptr;
    std::string semaphore_name_;
#endif
};
