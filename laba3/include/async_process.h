#include <iostream>
#include <string>
#include <cstdlib>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif


class AsyncProcess {
public:
    AsyncProcess(const std::string& program, const std::string& flag);

    bool start();
    bool check_status();
    void stop();

private:
    std::string program_;
    std::string flag_;
    pid_t pid_;
};