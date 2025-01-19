#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <ctime>
#include <chrono>
#include <cstring>
#include <pqxx/pqxx>
#include <type_traits>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <numeric>
#include "com_port.hpp"

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

const int PORT = 8080;
const int BUFFER_SIZE = 1024;

std::string getCurrentTemperature(pqxx::connection& conn) {
    try {
        pqxx::work txn(conn);
        pqxx::result res = txn.exec("SELECT temperature, timestamp FROM temperature_logs ORDER BY timestamp DESC LIMIT 1;");
        if (!res.empty()) {
            std::ostringstream response;
            response << "{ \"temperature\": " << res[0]["temperature"].as<double>()
                     << ", \"timestamp\": \"" << res[0]["timestamp"].as<std::string>() << "\" }";
            return response.str();
        }
        return "{}";
    } catch (const std::exception& e) {
        return std::string("{\"error\": \"") + e.what() + "\"}";
    }
}

std::string getAverageTemperature(pqxx::connection& conn, const std::string& type) {
    try {
        pqxx::work txn(conn);
        std::ostringstream query;
        query << "SELECT average, timestamp FROM average_temperatures WHERE type = '" << type
              << "' ORDER BY timestamp DESC LIMIT 1;";
        pqxx::result res = txn.exec(query.str());
        if (!res.empty()) {
            std::ostringstream response;
            response << "{ \"average\": " << res[0]["average"].as<double>()
                     << ", \"timestamp\": \"" << res[0]["timestamp"].as<std::string>() << "\" }";
            return response.str();
        }
        return "{}";
    } catch (const std::exception& e) {
        return std::string("{\"error\": \"") + e.what() + "\"}";
    }
}

std::string getAllTemperatures(pqxx::connection& conn) {
    try {
        pqxx::work txn(conn);
        pqxx::result res = txn.exec("SELECT temperature, timestamp FROM temperature_logs ORDER BY timestamp;");
        if (!res.empty()) {
            std::ostringstream response;
            response << "[";
            for (const auto& row : res) {
                response << "{ \"temperature\": " << row["temperature"].as<double>()
                         << ", \"timestamp\": \"" << row["timestamp"].as<std::string>() << "\" },";
            }
            std::string result = response.str();
            result.pop_back(); // Удалить последнюю запятую
            result += "]";
            return result;
        }
        return "[]";
    } catch (const std::exception& e) {
        return std::string("{\"error\": \"") + e.what() + "\"}";
    }
}

void handleClient(int clientSocket, pqxx::connection& conn) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

#ifdef _WIN32
    int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
#else
    int bytesReceived = read(clientSocket, buffer, BUFFER_SIZE);
#endif

    if (bytesReceived <= 0) {
        std::cerr << "Failed to receive data.\n";
        return;
    }

    std::string request(buffer);
    std::string response;

    if (request.find("GET /current_temperature") != std::string::npos) {
        response = getCurrentTemperature(conn);
    } else if (request.find("GET /hourly_avg") != std::string::npos) {
        response = getAverageTemperature(conn, "hourly");
    } else if (request.find("GET /daily_avg") != std::string::npos) {
        response = getAverageTemperature(conn, "daily");
    } else if (request.find("GET /all_temperatures") != std::string::npos) {
        response = getAllTemperatures(conn);
    } else {
        response = "{\"error\": \"404 Not Found\"}";
    }

    std::ostringstream httpResponse;
    httpResponse << "HTTP/1.1 200 OK\r\n"
                 << "Content-Type: application/json\r\n"
                 << "Content-Length: " << response.size() << "\r\n"
                 << "\r\n"
                 << response;

#ifdef _WIN32
    send(clientSocket, httpResponse.str().c_str(), httpResponse.str().size(), 0);
    closesocket(clientSocket);
#else
    write(clientSocket, httpResponse.str().c_str(), httpResponse.str().size());
    close(clientSocket);
#endif
}

void startServer(pqxx::connection& conn) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Socket creation failed.\n";
        return;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Bind failed.\n";
        return;
    }

    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Listen failed.\n";
        return;
    }

    std::cout << "Server is running on port " << PORT << "...\n";

    while (true) {
        sockaddr_in clientAddr{};
#ifdef _WIN32
        int clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
#else
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
#endif
        if (clientSocket < 0) {
            std::cerr << "Failed to accept client.\n";
            continue;
        }

        std::thread(handleClient, clientSocket, std::ref(conn)).detach();
    }

#ifdef _WIN32
    closesocket(serverSocket);
    WSACleanup();
#else
    close(serverSocket);
#endif
}

struct TemperatureData {
    double temperature;
    std::time_t timestamp;
};

// Запись данных температуры в PostgreSQL
void writeTemperatureToDB(pqxx::connection& conn, const TemperatureData& data) {
    try {
        pqxx::work txn(conn);

        std::ostringstream query;
        query << "INSERT INTO temperature_logs (temperature, timestamp) "
              << "VALUES (" << data.temperature << ", to_timestamp(" << data.timestamp << "));";

        txn.exec(query.str());
        txn.commit();
    } catch (const std::exception& e) {
        std::cerr << "Error writing to DB: " << e.what() << std::endl;
    }
}

// Запись среднесуточной или почасовой температуры в PostgreSQL
void writeAverageToDB(pqxx::connection& conn, const std::string& type, double avg, std::time_t timestamp) {
    try {
        pqxx::work txn(conn);

        std::ostringstream query;
        query << "INSERT INTO average_temperatures (type, average, timestamp) "
              << "VALUES ('" << type << "', " << avg << ", to_timestamp(" << timestamp << "));";

        txn.exec(query.str());
        txn.commit();
    } catch (const std::exception& e) {
        std::cerr << "Error writing average to DB: " << e.what() << std::endl;
    }
}

void logData(const std::string& filename, const std::vector<TemperatureData>& temperatureData, const std::string& data_to_log) {
    std::ofstream outFile(filename, std::ios_base::trunc);
    if (!outFile.is_open()) {
        throw std::runtime_error("Unable to open file");
    }

    char buffer[80];
    for (const auto& data : temperatureData) {
        std::tm* tm_info = std::localtime(&data.timestamp);
        std::strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", tm_info);
        outFile << buffer << " " << data_to_log << " " << data.temperature << std::endl;
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
    COMPort port("/dev/pts/12");
    port.open();
    pqxx::connection conn("host=localhost dbname=os user=postgres password=postgres");

    if (!conn.is_open()) {
        std::cerr << "Failed to connect to PostgreSQL.\n";
        return 1;
    }

    // Запуск сервера в отдельном потоке
    std::thread serverThread(startServer, std::ref(conn));
    serverThread.detach();


    std::vector<TemperatureData> temperature_vector;
    std::vector<TemperatureData> daily_avg_vector;
    std::vector<TemperatureData> hourly_avg_vector;

    std::time_t last_time_hour_log = std::time(NULL);
    std::time_t last_time_daily_log = std::time(NULL);


    while (true) {
        try {
            std::string str = port.read();
            double temperature = std::stod(str);
            std::time_t now = std::time(NULL);

            TemperatureData current_data = {temperature, now};
            temperature_vector.push_back(current_data);

            writeTemperatureToDB(conn, current_data);

            removeOldData(temperature_vector, 60 * 60 * 24);
            logData("temperature_log.txt", temperature_vector, "Temperature:");

            if (now - last_time_hour_log >= 60 * 60) {
                double hourly_avg_now = calculateAverageTemperature(temperature_vector, 60 * 60);
                hourly_avg_vector.push_back({hourly_avg_now, now});

                writeAverageToDB(conn, "hourly", hourly_avg_now, now);

                removeOldData(hourly_avg_vector, 24 * 30); // храним последний месяц
                logData("hourly_avg_log.txt", hourly_avg_vector, "Hourly avg temperature:");
                last_time_hour_log = now;
            }

            if (now - last_time_daily_log >= 60 * 60 * 24) {
                double daily_avg_now = calculateAverageTemperature(temperature_vector, 60 * 60 * 24);
                daily_avg_vector.push_back({daily_avg_now, now});

                writeAverageToDB(conn, "daily", daily_avg_now, now);

                removeOldData(daily_avg_vector, 365);
                logData("daily_avg_log.txt", daily_avg_vector, "Daily avg temperature:");
                last_time_daily_log = now;
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }


    return 0;
}