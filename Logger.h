#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <memory>

namespace CarRacing {
namespace Utils {

/**
 * @brief Log severity levels
 */
enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};

/**
 * @brief A thread-safe logging system
 * 
 * This class provides a centralized logging system that can output to
 * console and/or file, with different severity levels and timestamps.
 */
class Logger {
public:
    /**
     * @brief Get the singleton instance of the logger
     * 
     * @return Logger& The logger instance
     */
    static Logger& GetInstance() {
        static Logger instance;
        return instance;
    }

    /**
     * @brief Initialize the logger
     * 
     * @param logToConsole Whether to log to console
     * @param logToFile Whether to log to file
     * @param logFilePath The path to the log file (if logging to file)
     * @param minLevel The minimum log level to output
     */
    void Initialize(bool logToConsole = true, bool logToFile = false, 
                   const std::string& logFilePath = "game.log", 
                   LogLevel minLevel = LogLevel::Debug) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        logToConsole_ = logToConsole;
        logToFile_ = logToFile;
        minLevel_ = minLevel;
        
        if (logToFile_) {
            logFile_.open(logFilePath, std::ios::out | std::ios::app);
            if (!logFile_.is_open()) {
                std::cerr << "Failed to open log file: " << logFilePath << std::endl;
                logToFile_ = false;
            }
        }
        
        initialized_ = true;
        Log(LogLevel::Info, "Logger", "Logging system initialized");
    }

    /**
     * @brief Shut down the logger
     */
    void Shutdown() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (initialized_) {
            Log(LogLevel::Info, "Logger", "Logging system shutting down");
            
            if (logToFile_ && logFile_.is_open()) {
                logFile_.close();
            }
            
            initialized_ = false;
        }
    }

    /**
     * @brief Log a message
     * 
     * @param level The severity level of the message
     * @param tag The tag/category of the message
     * @param message The message to log
     */
    void Log(LogLevel level, const std::string& tag, const std::string& message) {
        if (!initialized_ || level < minLevel_) {
            return;
        }
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::stringstream logStream;
        logStream << GetTimestamp() << " [" << LogLevelToString(level) << "] "
                  << "[" << tag << "] " << message;
        
        std::string logMessage = logStream.str();
        
        if (logToConsole_) {
            SetConsoleColor(level);
            std::cout << logMessage << std::endl;
            ResetConsoleColor();
        }
        
        if (logToFile_ && logFile_.is_open()) {
            logFile_ << logMessage << std::endl;
            logFile_.flush();
        }
    }

    /**
     * @brief Log a debug message
     * 
     * @param tag The tag/category of the message
     * @param message The message to log
     */
    void Debug(const std::string& tag, const std::string& message) {
        Log(LogLevel::Debug, tag, message);
    }

    /**
     * @brief Log an info message
     * 
     * @param tag The tag/category of the message
     * @param message The message to log
     */
    void Info(const std::string& tag, const std::string& message) {
        Log(LogLevel::Info, tag, message);
    }

    /**
     * @brief Log a warning message
     * 
     * @param tag The tag/category of the message
     * @param message The message to log
     */
    void Warning(const std::string& tag, const std::string& message) {
        Log(LogLevel::Warning, tag, message);
    }

    /**
     * @brief Log an error message
     * 
     * @param tag The tag/category of the message
     * @param message The message to log
     */
    void Error(const std::string& tag, const std::string& message) {
        Log(LogLevel::Error, tag, message);
    }

    /**
     * @brief Log a fatal message
     * 
     * @param tag The tag/category of the message
     * @param message The message to log
     */
    void Fatal(const std::string& tag, const std::string& message) {
        Log(LogLevel::Fatal, tag, message);
    }

private:
    // Private constructor for singleton
    Logger() : initialized_(false), logToConsole_(true), logToFile_(false), minLevel_(LogLevel::Debug) {}
    
    // Prevent copying and moving
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;
    
    /**
     * @brief Get a timestamp string for the current time
     * 
     * @return std::string The timestamp string
     */
    std::string GetTimestamp() const {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
           << '.' << std::setfill('0') << std::setw(3) << ms.count();
        
        return ss.str();
    }
    
    /**
     * @brief Convert a log level to its string representation
     * 
     * @param level The log level
     * @return std::string The string representation
     */
    std::string LogLevelToString(LogLevel level) const {
        switch (level) {
            case LogLevel::Debug:   return "DEBUG";
            case LogLevel::Info:    return "INFO ";
            case LogLevel::Warning: return "WARN ";
            case LogLevel::Error:   return "ERROR";
            case LogLevel::Fatal:   return "FATAL";
            default:                return "UNKNOWN";
        }
    }
    
    /**
     * @brief Set the console color based on the log level
     * 
     * @param level The log level
     */
    void SetConsoleColor(LogLevel level) const {
        #ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        switch (level) {
            case LogLevel::Debug:   SetConsoleTextAttribute(hConsole, 8); break;  // Gray
            case LogLevel::Info:    SetConsoleTextAttribute(hConsole, 7); break;  // White
            case LogLevel::Warning: SetConsoleTextAttribute(hConsole, 14); break; // Yellow
            case LogLevel::Error:   SetConsoleTextAttribute(hConsole, 12); break; // Red
            case LogLevel::Fatal:   SetConsoleTextAttribute(hConsole, 79); break; // White on red
            default:                SetConsoleTextAttribute(hConsole, 7); break;  // White
        }
        #else
        switch (level) {
            case LogLevel::Debug:   std::cout << "\033[90m"; break; // Gray
            case LogLevel::Info:    std::cout << "\033[0m"; break;  // Default
            case LogLevel::Warning: std::cout << "\033[33m"; break; // Yellow
            case LogLevel::Error:   std::cout << "\033[31m"; break; // Red
            case LogLevel::Fatal:   std::cout << "\033[41m\033[37m"; break; // White on red
            default:                std::cout << "\033[0m"; break;  // Default
        }
        #endif
    }
    
    /**
     * @brief Reset the console color to default
     */
    void ResetConsoleColor() const {
        #ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, 7); // White
        #else
        std::cout << "\033[0m";
        #endif
    }

    bool initialized_;
    bool logToConsole_;
    bool logToFile_;
    LogLevel minLevel_;
    std::ofstream logFile_;
    std::mutex mutex_;
};

// Convenience macro for logging
#define LOG_DEBUG(tag, message) CarRacing::Utils::Logger::GetInstance().Debug(tag, message)
#define LOG_INFO(tag, message) CarRacing::Utils::Logger::GetInstance().Info(tag, message)
#define LOG_WARNING(tag, message) CarRacing::Utils::Logger::GetInstance().Warning(tag, message)
#define LOG_ERROR(tag, message) CarRacing::Utils::Logger::GetInstance().Error(tag, message)
#define LOG_FATAL(tag, message) CarRacing::Utils::Logger::GetInstance().Fatal(tag, message)

} // namespace Utils
} // namespace CarRacing