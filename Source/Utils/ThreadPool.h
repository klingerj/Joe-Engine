#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

namespace JoeEngine {
    // Sample data/function
    struct je_sample_data_t {
        std::string name;
        int id;
    };

    // Information needed for a job that a thread do
    // http://fabiensanglard.net/doom3_bfg/threading.php
    /*
     * Example usage:
       je_sample_data_t data = { "Work not done yet", -1 };
       void* dataPtr = &data;
       thread_job_t job = { &f, dataPtr, false }; // where f looks like: void f(void*)
    */
    typedef struct je_thread_job_t {
        void(*function)(void*);
        void* data;
        bool complete;
    } JEThreadJob;

    // https://stackoverflow.com/questions/15752659/thread-pooling-in-c11
    // Why not use std::async? https://eli.thegreenplace.net/2016/the-promises-and-challenges-of-stdasync-task-based-parallelism-in-c11/
    class JEThreadPool {
    private:
        bool m_quit; // marked true 
        std::mutex m_mutex_queue;
        std::condition_variable m_cv_queue;
        std::vector<std::thread> m_threads;
        std::queue<JEThreadJob> m_jobs;

        void ThreadFunction(); // Runs threads on an infinite loop until a job
        JEThreadJob DequeueJob(); // Atomically pop a job from the queue
        void ThreadDoJob(JEThreadJob threadJob); // Call the thread function on the thread's data
        void JoinThreads(); // All threads must rejoin the main thread before terminating the program
        void StopThreadJobs(); // All jobs will be removed from the queue, then all threads will return from ThreadFunction

    public:
        JEThreadPool() : m_quit(false) {
            for (uint32_t t = 0; t < std::thread::hardware_concurrency(); ++t) { // one less than main thread?
                m_threads.emplace_back(std::thread([this] { ThreadFunction(); }));
            }
        }
        ~JEThreadPool() {
            JoinThreads();
        }

        void EnqueueJob(JEThreadJob job); // Atomically queue up a new job
    };

    extern JEThreadPool JEthreadPool;
}
