#include "ThreadPool.h"

namespace JoeEngine {
    // Define extern threadpool object
    JEThreadPool JEthreadPool = JEThreadPool();

    // Atomically enqueue a new job
    void JEThreadPool::EnqueueJob(JEThreadJob job) {
        {
            std::unique_lock <std::mutex> lock(m_mutex_queue);
            m_jobs.push(job);
        }
        m_cv_queue.notify_one();
    }

    // Get a job from the queue - should be called while the queue mutex is locked
    JEThreadJob JEThreadPool::DequeueJob() {
        JEThreadJob job;
        job = m_jobs.front();
        m_jobs.pop();
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
                std::unique_lock<std::mutex> lock(m_mutex_queue);
                m_cv_queue.wait(lock, [this] { return !m_jobs.empty() || (m_jobs.empty() && m_quit); });
                if (m_quit) return;
                job = DequeueJob();
            }
            ThreadDoJob(job);
            job.complete = true;
        }
    }

    void JEThreadPool::JoinThreads() {
        StopThreadJobs();
        for (uint32_t i = 0; i < m_threads.size(); ++i) {
            m_threads[i].join();
        }
    }

    void JEThreadPool::StopThreadJobs() {
        m_quit = true;
        {
            std::unique_lock<std::mutex> lock(m_mutex_queue);
            while (!m_jobs.empty()) {
                m_jobs.pop();
            }
        }
        m_cv_queue.notify_all();
    }
}
