#pragma once

#include <chrono>
#include <string>
#include <iostream>

namespace JoeEngine {

    template <typename T>
    class ScopedTimer {
    private:
        using Clock = std::chrono::high_resolution_clock;
        using Time = std::chrono::time_point<Clock>;
        Time m_startTime;
        std::string m_str;
        
    public:
        ScopedTimer(const std::string& endMsg, const std::string& startMsg = "") : m_str(endMsg) {
            std::cout << startMsg;
            m_startTime = Clock::now();
        }
        ~ScopedTimer() {
            using Duration = std::chrono::duration<T, std::milli>;
            using Millis = std::chrono::milliseconds;
            Time endTime = Clock::now();
            Duration dur = endTime - m_startTime;
            std::cout << m_str << ": " << dur.count() << " ms" << std::endl;
        }
    };
}
