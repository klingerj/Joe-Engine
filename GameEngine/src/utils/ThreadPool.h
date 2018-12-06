#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>


// Sample data/function
struct sample_data_t {
    std::string name;
    int id;
};

// Information needed for a job that a thread do
// http://fabiensanglard.net/doom3_bfg/threading.php
/*
 * Example usage:
   sample_data_t data = { "Work not done yet", -1 };
   void* dataPtr = &data;
   thread_job_t job = { &f, dataPtr, false }; // where f looks like: void f(void*)
*/
struct thread_job_t {
    void(*function)(void*);
    void* data;
    bool complete;
};

// https://stackoverflow.com/questions/15752659/thread-pooling-in-c11
// Why not use std::async? https://eli.thegreenplace.net/2016/the-promises-and-challenges-of-stdasync-task-based-parallelism-in-c11/
class ThreadPool {
private:
    bool quit; // marked true 
    std::mutex mutex_queue;
    std::condition_variable cv_queue;
    std::vector<std::thread> threads;
    std::queue<thread_job_t> jobs;

    void ThreadFunction(); // Runs threads on an infinite loop until a job
    thread_job_t DequeueJob(); // Atomically pop a job from the queue
    void ThreadDoJob(thread_job_t threadJob); // Call the thread function on the thread's data
    void JoinThreads(); // All threads must rejoin the main thread before terminating the program
    void StopThreadJobs(); // All jobs will be removed from the queue, then all threads will return from ThreadFunction

public:
    ThreadPool() : quit(false) {
        for (uint32_t t = 0; t < std::thread::hardware_concurrency(); ++t) { // one less than main thread?
            threads.emplace_back(std::thread([this] { ThreadFunction(); }));
        }
    }
    ~ThreadPool() {
        JoinThreads();
    }

    void EnqueueJob(thread_job_t job); // Atomically queue up a new job
};

extern ThreadPool threadPool;
