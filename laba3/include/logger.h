#include <string>
#include <fstream>
#include <stdexcept>

class Logger {
public:
    explicit Logger(const std::string& logFileName);
    ~Logger();

    void log(const std::string& message);

private:
    std::string logFileName_;
    std::ofstream logFile_;

    std::string getCurrentTime();
    int getProcessID();
};


