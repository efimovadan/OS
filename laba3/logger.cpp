#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>  
#include <ctime>   

class Logger {
public:
    Logger(const std::string& logFileName) : logFileName_(logFileName) {
        logFile_.open(logFileName_, std::ios::out | std::ios::app);
        if (!logFile_) {
            throw std::runtime_error("Unable to open log file: " + logFileName_);
        }
    }

    ~Logger() {
        if (logFile_.is_open()) {
            logFile_.close();
        }
    }

    void log(const std::string& message) {
        if (logFile_.is_open()) {
            logFile_ << getCurrentTime() << " - " <<  "PID: " << getProcessID() << " - " << message << std::endl;
            logFile_.flush();
        }
    }

private:
    std::string logFileName_;
    std::ofstream logFile_;

    std::string getCurrentTime() {
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

    int getProcessID() {
#ifdef _WIN32
        return GetCurrentProcessId();
#else
        return getpid();
#endif
    }
};

