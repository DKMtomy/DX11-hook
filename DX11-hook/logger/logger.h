#ifndef LOGGER_H
#define LOGGER_H

#include "../pch.h"
#include <Windows.h>
#include <iostream>
#include <sstream>
#include <mutex>
#include <chrono>
#include <iomanip>

class Logger {
public:
    enum class LogLevel {
        INFO,
        DEBUG,
        WARN,
        CRITICAL
    };

    static void Init();

    static void Log(LogLevel level, const char* format, ...) {
        std::lock_guard<std::mutex> lock(mutex_);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7); // Color for <>

        std::string timestamp = GetTimestamp();
        std::string levelStr = GetLevelString(level);

        printf("<%s> [", timestamp.c_str());

        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GetColorForLevel(level)); // Color for log level
        printf("%s", levelStr.c_str());

        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7); // Color for <>
        printf("] ");

        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15); // Color for args (white)

        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);

        printf("\n");

        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7); // Reset to default color
    }

    static void Info(const char* format, ...) {
        va_list args;
        va_start(args, format);
        Log(LogLevel::INFO, format, args);
        va_end(args);
    }

    static void Debug(const char* format, ...) {
        va_list args;
        va_start(args, format);
        Log(LogLevel::DEBUG, format, args);
        va_end(args);
    }

    static void Warn(const char* format, ...) {
        va_list args;
        va_start(args, format);
        Log(LogLevel::WARN, format, args);
        va_end(args);
    }

    static void Critical(const char* format, ...) {
        va_list args;
        va_start(args, format);
        Log(LogLevel::CRITICAL, format, args);
        va_end(args);
    }
private:
    static std::mutex mutex_;

    static std::string GetTimestamp();
    static const char* GetLevelString(LogLevel level);
    static WORD GetColorForLevel(LogLevel level);
};

#endif // LOGGER_H
