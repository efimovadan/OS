#include <iostream>
#include <thread>
#include <string>

#include "mutex.cpp"
#include "shared_memory.cpp"
#include "logger.cpp"
#include "async_process.cpp"

void logger_thread(
    Logger& logger, 
    SharedMemoryInt& shared_memory, 
    CrossPlatformMutex& shared_mutex, 
    bool& working) 
{
    while(working) {
        std::stringstream ss;
        shared_mutex.lock();
        auto shared_value = shared_memory.getValue();
        shared_mutex.unlock();

        ss << "Value: " << shared_value;
        logger.log(ss.str());
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void counter_thread(
    SharedMemoryInt& shared_memory,
    CrossPlatformMutex& shared_mutex,
    bool& working) 
{
    while(working) {
        shared_mutex.lock();
        shared_memory.setValue(shared_memory.getValue() + 1);
        shared_mutex.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
}

void input_terminal_thread(SharedMemoryInt& shared_memory,
    CrossPlatformMutex& shared_mutex,
    bool& is_working)
{
    std::string input;
    int number = 0;

    while(is_working) {
        std::cout << "Enter a number for a counter or 'exit' for exit: ";
        std::getline(std::cin, input);
        if (input == "exit") {
            std::cout << "Exiting..." << std::endl;
            is_working = false;
            break;
        }

        std::stringstream ss(input);
        ss >> number;
        if (ss.fail() || !ss.eof()) {
            std::cout << "Input Error! Write a number or 'exit'." << std::endl;
        } else {
            shared_mutex.lock();
            shared_memory.setValue(number);
            shared_mutex.unlock();
            std::cout << "Counter set to " << number << std::endl;
        }
    }
}

void make_copies_thread (
    Logger& logger,
    SharedMemoryInt& shared_memory,
    CrossPlatformMutex& shared_mutex,
    bool& is_working,
    const std::string& program
    ) 
{
    while(is_working) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        AsyncProcess copy1(program, "copy_one"), copy2(program, "copy_two");
        copy1.start();
        copy2.start();
        std::this_thread::sleep_for(std::chrono::seconds(3));

        while (!copy1.check_status() || !copy2.check_status()) {
            logger.log("Copies doesn't ended...");
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
    }
}

int main(int argc, char* argv[]) {
    CrossPlatformMutex mutex("SharedMutex");
    CrossPlatformMutex writeAndMakeCopiesMutex("CanWriteMutex");
    SharedMemoryInt shared_memory("shared_int");
    Logger logger("application.log");
    bool is_working = true;

    if (argc > 2) {
        std::cout << "Invalid arguments count" << std::endl;
        exit(1);
    }

    // Копия
    if (argc == 2) {
        std::string copy_arg = argv[1];
        if (copy_arg == "copy_one") {
            logger.log("Copy 1 started!");
            mutex.lock();
            shared_memory.setValue(shared_memory.getValue() + 10);
            mutex.unlock();
            logger.log("Copy 1 ended!");
        }
        else if (copy_arg == "copy_two") {
            logger.log("Copy 2 started!");
            mutex.lock();
            shared_memory.setValue(shared_memory.getValue() *2);
            mutex.unlock();
            std::this_thread::sleep_for(std::chrono::seconds(2));

            mutex.lock();
            shared_memory.setValue(shared_memory.getValue() / 2);
            mutex.unlock();
            logger.log("Copy 2 ended!");
        }
        else
            std::cout <<"Wtf are you running?!" << std::endl;
        exit(0);
    }

    std::thread 
        counter_th(counter_thread, std::ref(shared_memory), std::ref(mutex), std::ref(is_working)),
        input_th(input_terminal_thread, std::ref(shared_memory), std::ref(mutex), std::ref(is_working));

    writeAndMakeCopiesMutex.lock();
    
    std::thread 
        logger_th(logger_thread, std::ref(logger), std::ref(shared_memory), std::ref(mutex), std::ref(is_working)),
        make_copies_th(make_copies_thread, std::ref(logger), std::ref(shared_memory), std::ref(mutex), std::ref(is_working), argv[0]);

        

    while(is_working) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    writeAndMakeCopiesMutex.unlock();
    std::cout << "Stopping the threads...\n";
    logger_th.join();
    make_copies_th.join();
    counter_th.join();
    input_th.join();
    return 0;
}