#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

int run_command(const char *command) {
    if (command == NULL) {
        fprintf(stderr, "Команда не может быть NULL\n");
        return -1;
    }

#ifdef _WIN32
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);

    if (!CreateProcessA(NULL, (LPSTR)command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        return -1; 
    }

    CloseHandle(pi.hThread);


    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD status;
    if (!GetExitCodeProcess(pi.hProcess, &status)) {
        CloseHandle(pi.hProcess);
        return -1; 
    }
    CloseHandle(pi.hProcess);
    
    return (int)status;
#else
    pid_t pid = fork();
    if (pid < 0) {
        perror("Ошибка создания процесса");
        return -1;
    }

    if (pid == 0) {
        execl("/bin/sh", "sh", "-c", command, (char *)NULL);
        perror("Ошибка выполнения команды");
        exit(EXIT_FAILURE);
    } else {
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("Ошибка ожидания процесса");
            return -1;
        }

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            fprintf(stderr, "Процесс завершился ненормально\n");
            return -1;
        }
    }
#endif
}
