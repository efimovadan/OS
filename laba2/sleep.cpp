#include <stdio.h>
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

int main() {
    std::cout << "Пауза на 3 секунды..." << std::endl;

#ifdef _WIN32
    Sleep(3000);  
#else
    sleep(3);
#endif

    std::cout << "Пауза завершена!" << std::endl;
    return 0;
}