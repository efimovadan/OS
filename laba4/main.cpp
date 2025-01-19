#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <type_traits>

#include "com_port.hpp"

std::string getCurrentTime() {
    std::time_t now = std::time(nullptr);
    char buf[20];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return std::string(buf);
}

std::time_t parseTime(const std::string& timeStr) {
    std::tm tm = {};
    sscanf(timeStr.c_str(), "%d-%d-%d %d:%d:%d",
           &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
           &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    tm.tm_year -= 1900;
    tm.tm_mon -= 1;
    return std::mktime(&tm);
}


struct TemperatureData {
    double temperature;
    std::time_t timestamp;
};

void logData(const std::string& filename, const std::vector<TemperatureData>& temperatureData, const std::string& data_to_log) {
    std::ofstream outFile(filename, std::ios_base::trunc);
    if (!outFile.is_open()) {
        throw std::runtime_error("Unable to open file");
    }

    char buffer[80];
    for (const auto& data : temperatureData) {
        std::tm* tm_info = std::localtime(&data.timestamp);
        std::strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", tm_info);
        outFile << buffer <<  " " << data_to_log << " " << data.temperature << std::endl;
    }

    outFile.close();
}


void removeOldData(std::vector<TemperatureData>& temperatureData, int offset) {
    if (temperatureData.size() > offset) {
        temperatureData.erase(temperatureData.begin(), temperatureData.end() - offset);
    }
}

double calculateAverageTemperature(const std::vector<TemperatureData>& temperatureData, int offset) {
    if (temperatureData.empty()) {
        return 0.0;
    }
    double sum = std::accumulate(temperatureData.end() - offset, temperatureData.end(), 0.0,
                                 [](double sum, const TemperatureData& entry) { return sum + entry.temperature; });
    return sum / offset;
}


int main() {
    COMPort port("COM2");
    //COMPort port("/dev/pts/4");
    port.open();

    std::vector<TemperatureData> temperature_vector;

    //в лог файл
    std::vector<TemperatureData> daily_avg_vector;
    std::vector<TemperatureData> hourly_avg_vector;
    
    std::time_t last_time_hour_log = std::time(NULL);
    std::time_t last_time_daily_log = std::time(NULL);


    while (true) {
        std::string str = port.read();
        double temperature = std::stod(str);
        std::time_t now = std::time(NULL);

        temperature_vector.push_back({temperature, now});
        removeOldData(temperature_vector, 60*60*24);
        logData("temperature_log.txt", temperature_vector, "Temperature:");

        if (now - last_time_hour_log >= 60*60) {
            double hourly_avg_now = calculateAverageTemperature(temperature_vector, 60*60);
            hourly_avg_vector.push_back({hourly_avg_now, now});
            removeOldData(hourly_avg_vector, 24 * 30); // храним только последний месяц
            logData("hourly_avg_log.txt", hourly_avg_vector, "Hourly avg temperature:");
            last_time_hour_log = now;
        }

        if (now - last_time_daily_log >= 60*60*24) {
            double daily_avg_now = calculateAverageTemperature(temperature_vector, 60*60*24);
            removeOldData(daily_avg_vector, 365); // храним только последний год
            logData("daily_avg_log.txt", daily_avg_vector, "Daily avg temperature:");
            last_time_daily_log = now;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));  
    }

    port.close();
    return 0;
}