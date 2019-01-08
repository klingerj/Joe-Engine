#include "ThreadPool.h"

// Define extern threadpool object
JEThreadPool JEthreadPool = JEThreadPool();

// Atomically enqueue a new job
void JEThreadPool::EnqueueJob(JEThreadJob job) {
    {
        std::unique_lock <std::mutex> lock(mutex_queue);
        jobs.push(job);
    }
    cv_queue.notify_one();
}

// Get a job from the queue - should be called while the queue mutex is locked
JEThreadJob JEThreadPool::DequeueJob() {
    JEThreadJob job;
    job = jobs.front();
    jobs.pop();
    return job;
}

void JEThreadPool::ThreadDoJob(JEThreadJob threadJob) {
    threadJob.function(threadJob.data);
}

// Infinite loop - thread only gets launched once.
void JEThreadPool::ThreadFunction() {
    while (true) {
        JEThreadJob job;
        {
            std::unique_lock<std::mutex> lock(mutex_queue);
            cv_queue.wait(lock, [this] { return !jobs.empty() || (jobs.empty() && quit); });
            if (quit) return;
            job = DequeueJob();
        }
        ThreadDoJob(job);
        job.complete = true;
    }
}

void JEThreadPool::JoinThreads() {
    StopThreadJobs();
    for (uint32_t i = 0; i < threads.size(); ++i) {
        threads[i].join();
    }
}

void JEThreadPool::StopThreadJobs() {
    quit = true;
    {
        std::unique_lock<std::mutex> lock(mutex_queue);
        while (!jobs.empty()) {
            jobs.pop();
        }
    }
    cv_queue.notify_all();
}
