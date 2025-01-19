#include "com_port.hpp"
#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <random>

//std::string PORT = "COM1";
std::string PORT = "/dev/pts/11";

int main(void) {
    COMPort port (PORT);
    port.open();
    std::cout << "Temperature Generator started" << std::endl;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-20.0, 40.0);

    while(1) {
        double temp = dis(gen);
        if (!port.write(std::to_string(temp) + '\n')) {
            std::cout << "Failed to write :(\n";
        }
        std::cout << "Temp: " << temp << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}