#include "com_port.hpp"
#include <iostream>
#include <vector>

COMPort::COMPort(const std::string& portName) : portName(portName) {
#ifdef _WIN32
    hSerial = INVALID_HANDLE_VALUE;
#else
    fd = -1;
#endif
}

COMPort::~COMPort() {
    close();
}

bool COMPort::open() {
#ifdef _WIN32
    hSerial = CreateFileA(
        portName.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Error opening COM port: " << GetLastError() << std::endl;
        return false;
    }

    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error getting COM port state: " << GetLastError() << std::endl;
        close();
        return false;
    }

    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error setting COM port state: " << GetLastError() << std::endl;
        close();
        return false;
    }

    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        std::cerr << "Error setting COM port timeouts: " << GetLastError() << std::endl;
        close();
        return false;
    }
#else
    fd = ::open(portName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("Error opening COM port");
        return false;
    }

    termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        perror("Error getting COM port attributes");
        close();
        return false;
    }

    cfsetospeed(&tty, B9600);
    cfsetispeed(&tty, B9600);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag = IGNPAR;
    tty.c_oflag = 0;
    tty.c_lflag = 0;

    tcflush(fd, TCIFLUSH);
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("Error setting COM port attributes");
        close();
        return false;
    }
#endif
    return true;
}

void COMPort::close() {
#ifdef _WIN32
    if (hSerial != INVALID_HANDLE_VALUE) {
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
    }
#else
    if (fd != -1) {
        ::close(fd);
        fd = -1;
    }
#endif
}

bool COMPort::write(const std::string& data) {

    std::string wrappedData = "{" + data + "}";
#ifdef _WIN32
    DWORD bytes;
    if (!WriteFile(hSerial, wrappedData.c_str(), wrappedData.length(), &bytes, NULL)) {
        std::cerr << "Error writing to COM port: " << GetLastError() << std::endl;
        return false;
    }
#else
    ssize_t bytes = ::write(fd, wrappedData.c_str(), wrappedData.length());
    if (bytes < 0) {
        perror("Error writing to COM port");
        return false;
    }
#endif
    return true;
}

std::string COMPort::read() {
    bool endOfMessage = false;
    std::string out;
    char buf[1];
    bool insideBrackets = false;

#ifdef _WIN32
    DWORD bytes;
    while (!endOfMessage) {
        if (!ReadFile(hSerial, buf, 1, &bytes, NULL)) {
            continue;
        }
        if (bytes > 0) {
            char c = buf[0];
            if (c == '{') {
                insideBrackets = true;
                out.clear();
            } else if (c == '}' && insideBrackets) {
                endOfMessage = true;
            } else if (insideBrackets) {
                out += c;
            }
        }
    }
#else
    ssize_t bytes;
    while (!endOfMessage) {
        bytes = ::read(fd, buf, 1);
        if (bytes < 0) {
            continue;
        }
        if (bytes > 0) {
            char c = buf[0];
            if (c == '{') {
                insideBrackets = true;
                out.clear();
            } else if (c == '}' && insideBrackets) {
                endOfMessage = true;
            } else if (insideBrackets) {
                out += c;
            }
        }
    }
#endif
    return out;
}
