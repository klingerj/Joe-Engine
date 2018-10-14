#include <stdio.h>
#include <iostream>
#include <string>
#include <future>
#include <thread>
#include <chrono>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "EngineApplication.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

int RunApp() {
    try {
        EngineApplication app = EngineApplication();
        app.Run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

struct sample_data_t {
    std::string name;
    int id;
};

std::atomic<long long int> sumSum = 0;
void ManipulateData(void* data) {
    sample_data_t* sampleData = static_cast<sample_data_t*>(data);
    long long int sum = 0;
    for (int i = 0; i < 8000000; ++i) {
        sum += sampleData->id * i;
    }
    sumSum += sum;
}

struct thread_job_t {
    void(*ManipulateData)(void*);
    void* data;
};

void ThreadDoJob_NoClass(thread_job_t threadJob) {
    threadJob.ManipulateData(threadJob.data);
}

std::mutex mutex_threadsFinished;
std::condition_variable cv_threadsFinished;

// https://stackoverflow.com/questions/15752659/thread-pooling-in-c11
// Why not use std::async? https://eli.thegreenplace.net/2016/the-promises-and-challenges-of-stdasync-task-based-parallelism-in-c11/
class ThreadPool {
private:
    bool quit;
    std::mutex mutex_queue;
    std::condition_variable cv_queue;
    std::vector<std::thread> threads;
    std::queue<thread_job_t> jobs;

    void ThreadFunction(); // Runs threads on an infinite loop until a job

public:
    ThreadPool() : quit(false) {
        for (unsigned int t = 0; t < std::thread::hardware_concurrency(); ++t) { // one less than main thread?
            threads.emplace_back(std::thread([this] { ThreadFunction(); }));
        }
    }
    ~ThreadPool() {}

    void EnqueueJob(thread_job_t job);
    thread_job_t DequeueJob();
    void ThreadDoJob(thread_job_t threadJob);
    void JoinThreads();
    void StopThreads();
    size_t NumJobsRemaining() {
        size_t numJobs = -1;
        {
            std::unique_lock<std::mutex> lock(mutex_queue);
            numJobs = jobs.size();
        }
        return numJobs;
    }
};

void ThreadPool::JoinThreads() {
    StopThreads();
    for (unsigned int i = 0; i < threads.size(); ++i) {
        threads[i].join();
    }
}

void ThreadPool::StopThreads() {
    quit = true;
    {
        std::unique_lock<std::mutex> lock(mutex_queue);
        while (!jobs.empty()) {
            jobs.pop();
        }
    }
    cv_queue.notify_all();
}

// Atomically enqueue a new job
void ThreadPool::EnqueueJob(thread_job_t job) {
    {
        std::unique_lock <std::mutex> lock(mutex_queue);
        jobs.push(job);
    }
    cv_queue.notify_one();
}

thread_job_t ThreadPool::DequeueJob() {
    thread_job_t job;
    job = jobs.front();
    jobs.pop();
    if (jobs.empty()) {
        cv_threadsFinished.notify_one();
    }
    return job;
}

// Do the job
void ThreadPool::ThreadDoJob(thread_job_t threadJob) {
    threadJob.ManipulateData(threadJob.data);
}

// Infinite loop - thread only gets launched once.
void ThreadPool::ThreadFunction() {
    while (true) { 
        thread_job_t job;
        {
            std::unique_lock<std::mutex> lock(mutex_queue);
            cv_queue.wait(lock, [this] { return !jobs.empty() || (jobs.empty() && quit); });
            if (quit) return;
            job = DequeueJob();
        }
        ThreadDoJob(job);
    }
}

void DoThreadStuff() {
    // Multithreading testing

    // http://fabiensanglard.net/doom3_bfg/threading.php
    sample_data_t data = { "Work not done yet", -1 };
    void* dataPtr = &data;
    thread_job_t job = { &ManipulateData, dataPtr };
    constexpr int numTimes = 10000;

    // just async
    std::cout << "Starting Async benchmark" << std::endl;
    std::future<void> futures_async[numTimes];
    auto startTime = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numTimes; ++i) {
        futures_async[i] = std::async(std::launch::async, ThreadDoJob_NoClass, job);
    }
    for (int i = 0; i < numTimes; ++i) {
        futures_async[i].wait();
    }
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    std::cout << "Async time: " << time << std::endl;

    std::vector<int> numjobs = std::vector<int>();
    std::vector<int> numjobs_during = std::vector<int>();
    // threads
    std::cout << "Starting threadpool benchmark" << std::endl;
    startTime = std::chrono::high_resolution_clock::now();
    ThreadPool threadPool = ThreadPool();
    for (int i = 0; i < numTimes; ++i) {
        threadPool.EnqueueJob(job);
        if (i % 200 == 0) {
            numjobs.push_back(threadPool.NumJobsRemaining());
        }
    }
    /*std::this_thread::sleep_for(std::chrono::seconds(1));
    for (int i = 0; i < 4; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        numjobs_during.push_back(threadPool.NumJobsRemaining());
    }*/
    {
        std::unique_lock<std::mutex> lock(mutex_threadsFinished);
        cv_threadsFinished.wait(lock);
    }
    threadPool.JoinThreads();
    currentTime = std::chrono::high_resolution_clock::now();
    time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    std::cout << "Thread time: " << time << std::endl;

    // single thread
    /*std::cout << "Starting single thread benchmark" << std::endl;
    startTime = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numTimes; ++i) {
        ThreadDoJob_NoClass(job);
    }
    currentTime = std::chrono::high_resolution_clock::now();
    time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    std::cout << "Single thread time: " << time << std::endl;*/

    std::cout << "Result: " << sumSum << std::endl;
    std::cout << "Jobs: " << std::endl;
    for (int j : numjobs_during) {
        std::cout << "NumJobs: " << j << std::endl;
    }
}

int main() {
    //return RunApp();
    DoThreadStuff();
    return EXIT_SUCCESS;
}
