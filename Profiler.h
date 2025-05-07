#pragma once

#include <string>
#include <chrono>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <memory>
#include <functional>

namespace CarRacing {
namespace Utils {

/**
 * @brief A simple profiling system for performance measurement
 * 
 * This class provides tools for measuring the execution time of code sections,
 * collecting statistics, and generating reports.
 */
class Profiler {
public:
    /**
     * @brief Get the singleton instance of the profiler
     * 
     * @return Profiler& The profiler instance
     */
    static Profiler& GetInstance() {
        static Profiler instance;
        return instance;
    }

    /**
     * @brief Initialize the profiler
     * 
     * @param enabled Whether the profiler is enabled
     */
    void Initialize(bool enabled = true) {
        std::lock_guard<std::mutex> lock(mutex_);
        enabled_ = enabled;
        profileData_.clear();
    }

    /**
     * @brief Begin profiling a section of code
     * 
     * @param name The name of the section
     */
    void BeginProfile(const std::string& name) {
        if (!enabled_) return;
        
        std::lock_guard<std::mutex> lock(mutex_);
        auto& data = profileData_[name];
        data.activeCount++;
        data.startTimes.push_back(std::chrono::high_resolution_clock::now());
    }

    /**
     * @brief End profiling a section of code
     * 
     * @param name The name of the section
     */
    void EndProfile(const std::string& name) {
        if (!enabled_) return;
        
        auto endTime = std::chrono::high_resolution_clock::now();
        
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = profileData_.find(name);
        if (it == profileData_.end() || it->second.activeCount == 0 || it->second.startTimes.empty()) {
            return;
        }
        
        auto& data = it->second;
        auto startTime = data.startTimes.back();
        data.startTimes.pop_back();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
        data.totalTime += duration;
        data.callCount++;
        data.activeCount--;
        
        data.minTime = data.callCount == 1 ? duration : std::min(data.minTime, duration);
        data.maxTime = data.callCount == 1 ? duration : std::max(data.maxTime, duration);
    }

    /**
     * @brief Reset all profiling data
     */
    void Reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        profileData_.clear();
    }

    /**
     * @brief Generate a report of profiling data
     * 
     * @param sortBy How to sort the report (0 = by name, 1 = by total time, 2 = by average time)
     * @return std::string The report as a string
     */
    std::string GenerateReport(int sortBy = 1) const {
        if (!enabled_) return "Profiler is disabled";
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Collect data for sorting
        std::vector<std::pair<std::string, ProfileData>> sortedData;
        for (const auto& pair : profileData_) {
            sortedData.push_back(pair);
        }
        
        // Sort the data
        switch (sortBy) {
            case 0: // Sort by name
                std::sort(sortedData.begin(), sortedData.end(),
                    [](const auto& a, const auto& b) { return a.first < b.first; });
                break;
            case 1: // Sort by total time (default)
                std::sort(sortedData.begin(), sortedData.end(),
                    [](const auto& a, const auto& b) { return a.second.totalTime > b.second.totalTime; });
                break;
            case 2: // Sort by average time
                std::sort(sortedData.begin(), sortedData.end(),
                    [](const auto& a, const auto& b) {
                        double avgA = a.second.callCount > 0 ? a.second.totalTime / static_cast<double>(a.second.callCount) : 0;
                        double avgB = b.second.callCount > 0 ? b.second.totalTime / static_cast<double>(b.second.callCount) : 0;
                        return avgA > avgB;
                    });
                break;
        }
        
        // Generate the report
        std::stringstream ss;
        ss << "======== Profiling Report ========\n";
        ss << std::left << std::setw(30) << "Section" 
           << std::right << std::setw(10) << "Calls" 
           << std::right << std::setw(15) << "Total (ms)" 
           << std::right << std::setw(15) << "Avg (ms)" 
           << std::right << std::setw(15) << "Min (ms)" 
           << std::right << std::setw(15) << "Max (ms)" << "\n";
        ss << std::string(100, '-') << "\n";
        
        for (const auto& pair : sortedData) {
            const auto& name = pair.first;
            const auto& data = pair.second;
            
            double totalMs = data.totalTime / 1000.0;
            double avgMs = data.callCount > 0 ? totalMs / data.callCount : 0;
            double minMs = data.minTime / 1000.0;
            double maxMs = data.maxTime / 1000.0;
            
            ss << std::left << std::setw(30) << name 
               << std::right << std::setw(10) << data.callCount 
               << std::right << std::setw(15) << std::fixed << std::setprecision(3) << totalMs 
               << std::right << std::setw(15) << std::fixed << std::setprecision(3) << avgMs 
               << std::right << std::setw(15) << std::fixed << std::setprecision(3) << minMs 
               << std::right << std::setw(15) << std::fixed << std::setprecision(3) << maxMs << "\n";
        }
        
        ss << "==================================\n";
        return ss.str();
    }

    /**
     * @brief Save the profiling report to a file
     * 
     * @param filename The name of the file to save to
     * @param sortBy How to sort the report (0 = by name, 1 = by total time, 2 = by average time)
     * @return bool Whether the save was successful
     */
    bool SaveReportToFile(const std::string& filename, int sortBy = 1) const {
        if (!enabled_) return false;
        
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        file << GenerateReport(sortBy);
        return true;
    }

    /**
     * @brief Enable or disable the profiler
     * 
     * @param enabled Whether the profiler should be enabled
     */
    void SetEnabled(bool enabled) {
        std::lock_guard<std::mutex> lock(mutex_);
        enabled_ = enabled;
    }

    /**
     * @brief Check if the profiler is enabled
     * 
     * @return bool Whether the profiler is enabled
     */
    bool IsEnabled() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return enabled_;
    }

private:
    // Private constructor for singleton
    Profiler() : enabled_(false) {}
    
    // Prevent copying and moving
    Profiler(const Profiler&) = delete;
    Profiler& operator=(const Profiler&) = delete;
    Profiler(Profiler&&) = delete;
    Profiler& operator=(Profiler&&) = delete;
    
    /**
     * @brief Data structure for storing profiling information
     */
    struct ProfileData {
        std::vector<std::chrono::high_resolution_clock::time_point> startTimes;
        int64_t totalTime = 0;    // Total time in microseconds
        int64_t minTime = 0;      // Minimum time in microseconds
        int64_t maxTime = 0;      // Maximum time in microseconds
        uint32_t callCount = 0;   // Number of times this section was profiled
        uint32_t activeCount = 0; // Number of active profiling sessions for this section
    };
    
    bool enabled_;
    std::unordered_map<std::string, ProfileData> profileData_;
    mutable std::mutex mutex_;
};

/**
 * @brief RAII wrapper for profiling a scope
 * 
 * This class automatically begins profiling when constructed and
 * ends profiling when destroyed, making it easy to profile a scope.
 */
class ScopedProfiler {
public:
    /**
     * @brief Construct a new Scoped Profiler object
     * 
     * @param name The name of the section to profile
     */
    explicit ScopedProfiler(const std::string& name) : name_(name) {
        Profiler::GetInstance().BeginProfile(name_);
    }
    
    /**
     * @brief Destroy the Scoped Profiler object
     * 
     * Automatically ends the profiling session.
     */
    ~ScopedProfiler() {
        Profiler::GetInstance().EndProfile(name_);
    }
    
    // Prevent copying and moving
    ScopedProfiler(const ScopedProfiler&) = delete;
    ScopedProfiler& operator=(const ScopedProfiler&) = delete;
    ScopedProfiler(ScopedProfiler&&) = delete;
    ScopedProfiler& operator=(ScopedProfiler&&) = delete;
    
private:
    std::string name_;
};

// Convenience macros for profiling
#ifdef ENABLE_PROFILING
    #define PROFILE_FUNCTION() CarRacing::Utils::ScopedProfiler profiler__(__FUNCTION__)
    #define PROFILE_SCOPE(name) CarRacing::Utils::ScopedProfiler profiler__(name)
    #define PROFILE_BEGIN(name) CarRacing::Utils::Profiler::GetInstance().BeginProfile(name)
    #define PROFILE_END(name) CarRacing::Utils::Profiler::GetInstance().EndProfile(name)
#else
    #define PROFILE_FUNCTION()
    #define PROFILE_SCOPE(name)
    #define PROFILE_BEGIN(name)
    #define PROFILE_END(name)
#endif

} // namespace Utils
} // namespace CarRacing