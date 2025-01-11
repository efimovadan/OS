#include "background.cpp"
int main(void) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    run_command("./sleep.exe");
#else
    run_command("./sleep")
#endif
    return 0;
}