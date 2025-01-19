#include "async_process.h"
#include <iostream>
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

AsyncProcess::AsyncProcess(const std::string& program, const std::string& flag) 
    : program_(program), flag_(flag), pid_(-1) {}

bool AsyncProcess::start() {
#ifdef _WIN32
    std::string command = program_ + " " + flag_;
    STARTUPINFOA si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;
    if (!CreateProcessA(nullptr, (LPSTR)command.c_str(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
        std::cerr << "Error starting process: " << GetLastError() << std::endl;
        return false;
    }
    pid_ = pi.dwProcessId;
#else
    pid_ = fork();
    if (pid_ == 0) {
        execlp(program_.c_str(), program_.c_str(), flag_.c_str(), nullptr);
        std::cerr << "Error executing program" << std::endl;
        exit(1);  
    } else if (pid_ < 0) {
        std::cerr << "Error forking process" << std::endl;
        return false;
    }
#endif
    return true;
}

bool AsyncProcess::check_status() {
    if (pid_ == -1) {
        std::cerr << "Process not started" << std::endl;
        return false;
    }

#ifdef _WIN32
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid_);
    if (hProcess == nullptr) {
        std::cerr << "Failed to open process: " << GetLastError() << std::endl;
        return false;
    }

    DWORD exitCode;
    if (!GetExitCodeProcess(hProcess, &exitCode)) {
        std::cerr << "Failed to get exit code: " << GetLastError() << std::endl;
        return false;
    }

    return exitCode != STILL_ACTIVE;
#else
    int status;
    pid_t result = waitpid(pid_, &status, WNOHANG);
    if (result == 0) {
        return false;
    } else if (result == pid_) {
        return true;
    } else {
        std::cerr << "Error checking process status" << std::endl;
        return false;
    }
#endif
}

void AsyncProcess::stop() {
    if (pid_ == -1) return;
#ifdef _WIN32
    TerminateProcess(OpenProcess(PROCESS_TERMINATE, FALSE, pid_), 0);
#else
    kill(pid_, SIGKILL);
#endif
}
