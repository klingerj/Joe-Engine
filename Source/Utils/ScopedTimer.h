#pragma once

#include <chrono>
#include <string>
#include <iostream>

namespace JoeEngine {
    //! The ScopedTimer class
    /*!
      Class dedicated to finding the elapsed time of a segment of code.
      It is meant to be used as follows, using temporary scopes:\n
      <pre>
      {\n
        ScopedTimer<float> timer;\n
        (code to profile)\n
      }
      </pre>
    */
    template <typename T>
    class ScopedTimer {
    private:
        using Clock = std::chrono::high_resolution_clock;
        using Time = std::chrono::time_point<Clock>;

        //! Start time for profiling.
        /*! Populated upon construction of the class. */
        Time m_startTime;

        //! Message string.
        /*! String to print after profiling. */
        std::string m_str;
        
    public:
        //! Default constructor.
        /*! Deleted. */
        ScopedTimer() = delete;

        //! Constructor.
        /*! Prints the start message and initializes the start time member. */
        ScopedTimer(const std::string& endMsg, const std::string& startMsg = "") : m_str(endMsg) {
            std::cout << startMsg;
            m_startTime = Clock::now();
        }

        //! Destructor.
        /*! Calculates the elapsed time since construction and prints the end message. */
        ~ScopedTimer() {
            using Duration = std::chrono::duration<T, std::milli>;
            using Millis = std::chrono::milliseconds;
            Time endTime = Clock::now();
            Duration dur = endTime - m_startTime;
            std::cout << m_str << ": " << dur.count() << " ms" << std::endl;
        }
    };
}
