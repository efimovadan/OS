#ifndef COM_PORT_H
#define COM_PORT_H

#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif

class COMPort {
public:
    COMPort(const std::string& portName);
    ~COMPort();

    bool open();
    void close();
    bool write(const std::string& data);
    std::string read();

private:
    std::string portName;
#ifdef _WIN32
    HANDLE hSerial;
#else
    int fd;
#endif
};

#endif // COM_PORT_H