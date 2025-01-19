#include "background.hpp"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

int main(void) {
    int exit_code;
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    exit_code = run_command("./sleep.exe");
#else
    exit_code = run_command("./sleep");
#endif
    std::cout << "Код выхода программы: " << exit_code << std::endl;
    return 0;
}