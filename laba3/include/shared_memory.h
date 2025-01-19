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
    explicit SharedMemoryInt(const std::string& name);
    ~SharedMemoryInt();

    void setValue(int value);
    int getValue() const;

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