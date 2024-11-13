#include "Logger.h"
#include <iostream>
#include <fstream>
#include <cerrno>
#include <cstring>
#include <sys/stat.h>

Logger::Logger(const std::string& filename, size_t maxFileSize)
    : logFileName(filename), maxFileSize(maxFileSize) {
    
    std::filesystem::path filePath(filename);
    std::filesystem::path dirPath = filePath.parent_path();
    
    if (!std::filesystem::exists(dirPath)) {
        std::cerr << "Directory does not exist. Creating directory: " << dirPath << std::endl;
        std::filesystem::create_directories(dirPath);
    }

    logFile.open(filename, std::ios::out | std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file: " << filename << std::endl;
        std::cerr << "Error: " << strerror(errno) << std::endl;
        exit(1);
    }
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    if (logFile.is_open()) {
        rotateLog();
        logFile << logLevelToString(level) << ": " << message << std::endl;
    } else {
        std::cerr << "Log file not open!" << std::endl;
    }
}

void Logger::log(const std::string& message) {
    if (logFile.is_open()) {
        rotateLog();
        logFile << message << std::endl;
    } else {
        std::cerr << "Log file not open!" << std::endl;
    }
}

bool Logger::checkFileSize() {
    struct stat stat_buf;
    if (stat(logFileName.c_str(), &stat_buf) == 0) {
        return stat_buf.st_size >= maxFileSize;
    }
    return false;
}

void Logger::rotateLog() {
    if (checkFileSize()) {
        logFile.close();
        std::string newLogFileName = logFileName + ".1";
        std::rename(logFileName.c_str(), newLogFileName.c_str());
        logFile.open(logFileName, std::ios::out | std::ios::app);
    }
}

const char* Logger::logLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}
