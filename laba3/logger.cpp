#include "logger.h"
#include <chrono>
#include <iomanip>
#include <ctime>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

Logger::Logger(const std::string& logFileName) : logFileName_(logFileName) {
    logFile_.open(logFileName_, std::ios::out | std::ios::app);
    if (!logFile_) {
        throw std::runtime_error("Unable to open log file: " + logFileName_);
    }
}

Logger::~Logger() {
    if (logFile_.is_open()) {
        logFile_.close();
    }
}

void Logger::log(const std::string& message) {
    if (logFile_.is_open()) {
        logFile_ << getCurrentTime() << " - " << "PID: " << getProcessID() << " - " << message << std::endl;
        logFile_.flush();
    }
}

std::string Logger::getCurrentTime() {
    using namespace std::chrono;

    auto now = system_clock::now();
    auto now_c = system_clock::to_time_t(now);
    auto duration = now.time_since_epoch();
    auto millisecond = duration_cast<milliseconds>(duration).count() % 1000;

    std::tm tm = *std::localtime(&now_c);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(3) << millisecond;

    return oss.str();
}

int Logger::getProcessID() {
#ifdef _WIN32
    return GetCurrentProcessId();
#else
    return getpid();
#endif
}
