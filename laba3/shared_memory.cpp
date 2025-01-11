#include <iostream>
#include <string>
#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
#else
#include <cstring>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#endif

class SharedMemoryInt {
public:
    explicit SharedMemoryInt(const std::string& name) : name_(name) {
#ifdef _WIN32
        hMapFile_ = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(int), name_.c_str());
        if (hMapFile_ == nullptr) {
            throw std::runtime_error("Failed to create/open shared memory: " + std::to_string(GetLastError()));
        }

        ptr_ = MapViewOfFile(hMapFile_, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int));
        if (ptr_ == nullptr) {
            throw std::runtime_error("Failed to map shared memory: " + std::to_string(GetLastError()));
        }
#else
        shm_fd_ = shm_open(("/" + name).c_str(), O_CREAT | O_RDWR, 0666);
        if (shm_fd_ == -1) {
            throw std::runtime_error("Failed to open shared memory: " + std::string(strerror(errno)));
        }

        if (ftruncate(shm_fd_, sizeof(int)) == -1) {
            throw std::runtime_error("Failed to set size for shared memory: " + std::string(strerror(errno)));
        }

        ptr_ = mmap(nullptr, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_, 0);
        if (ptr_ == MAP_FAILED) {
            throw std::runtime_error("Failed to map shared memory: " + std::string(strerror(errno)));
        }
#endif
    }

    ~SharedMemoryInt() {
#ifdef _WIN32
        if (ptr_) {
            UnmapViewOfFile(ptr_);
        }
        if (hMapFile_) {
            CloseHandle(hMapFile_);
        }
#else
        if (ptr_ != MAP_FAILED) {
            munmap(ptr_, sizeof(int));
        }
        if (shm_fd_ != -1) {
            close(shm_fd_);
            shm_unlink(("/" + name_).c_str());
        }
#endif
    }

    void setValue(int value) {
        *static_cast<int*>(ptr_) = value;
    }

    int getValue() const {
        return *static_cast<int*>(ptr_);
    }

private:
    std::string name_;
#ifdef _WIN32
    HANDLE hMapFile_ = nullptr;
    void* ptr_ = nullptr;
#else
    int shm_fd_ = -1;
    void* ptr_ = nullptr;
#endif
};
