#include "Logger.h"

std::mutex Logger::mutex_;

void Logger::Init() {
    AllocConsole();
    FILE* fDummy;
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
    SetConsoleTitle("Debug Console");
}

std::string Logger::GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::tm tm;
    localtime_s(&tm, &in_time_t);

    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %X");
    return ss.str();
}

const char* Logger::GetLevelString(LogLevel level) {
    switch (level) {
    case LogLevel::INFO:     return "INFO";
    case LogLevel::DEBUG:    return "DEBUG";
    case LogLevel::WARN:     return "WARN";
    case LogLevel::CRITICAL: return "CRITICAL";
    default:                 return "UNKNOWN";
    }
}

WORD Logger::GetColorForLevel(LogLevel level) {
    switch (level) {
    case LogLevel::INFO:     return 10; // Green
    case LogLevel::DEBUG:    return 11; // Cyan
    case LogLevel::WARN:     return 14; // Yellow
    case LogLevel::CRITICAL: return 12; // Red
    default:                 return 7;  // White (default)
    }
}
