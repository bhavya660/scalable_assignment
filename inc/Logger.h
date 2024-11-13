#include <string>
#include <fstream>
#include <filesystem>

class Logger {
public:
    enum class LogLevel { DEBUG, INFO, WARN, ERROR, FATAL };

    Logger(const std::string& filename, size_t maxFileSize);
    ~Logger();

    void log(LogLevel level, const std::string& message);
    void log(const std::string& message);

    
    template<typename... Args>
    void log(LogLevel level, const std::string& formatStr, Args... args);

private:
    std::ofstream logFile;
    std::string logFileName;
    size_t maxFileSize;

    bool checkFileSize();
    void rotateLog();
    const char* logLevelToString(LogLevel level);
};

// Template definition
template<typename... Args>
void Logger::log(LogLevel level, const std::string& formatStr, Args... args) {
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), formatStr.c_str(), args...);
    log(level, std::string(buffer));
}
