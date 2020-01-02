#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

namespace JoeEngine {
    // Sample data/function
    /*struct je_sample_data_t {
        std::string name;
        int id;
    };*/

    // Information needed for a job that a thread do
    // http://fabiensanglard.net/doom3_bfg/threading.php
    /*
     * Example usage:
       je_sample_data_t data = { "Work not done yet", -1 };
       void* dataPtr = &data;
       thread_job_t job = { &f, dataPtr, false }; // where f looks like: void f(void*)
    */
    //! Thread Job struct
    /*!
      Data for executing any threaded task.
    */
    typedef struct je_thread_job_t {
        void(*function)(void*);
        void* data;
        bool complete;
    } JEThreadJob;

    // https://stackoverflow.com/questions/15752659/thread-pooling-in-c11
    // Why not use std::async? https://eli.thegreenplace.net/2016/the-promises-and-challenges-of-stdasync-task-based-parallelism-in-c11/
    //! The JEThreadPool class
    /*!
      Class that manages any number of threads and a queue of tasks to complete. Threads are created and launched upon construction.
      Threads run in an infinite loop, block until the queue is populated with a task (or multiple), then execute the task.
    */
    class JEThreadPool {
    private:
        //! Queue access mutex.
        /*! Synchronizes threads pushing/popping jobs on/off the queue. */
        std::mutex m_mutex_queue;

        //! Queue access condition variable.
        /*! Condition variable that is waiting on by each thread in the pool. Notified when a job is added to the queue. */
        std::condition_variable m_cv_queue;

        //! List of threads.
        /*! List of each thread in the pool. */
        std::vector<std::thread> m_threads;

        //! Queue of thread jobs.
        /*! Queue of each job for the pool of threads to execute. */
        std::queue<JEThreadJob> m_jobs;

        //! Threadpool stopping flag.
        /*! Member flag that is set to true when the thread pool should be destroyed. */
        bool m_quit;

        //! Thread execution function.
        /*! Function that each thread in the pool executes. Synchronously obtains and executes jobs from the job queue. */
        void ThreadFunction(); // Runs threads on an infinite loop until a job

        //! Dequeue thread job.
        /*!
          Removes a job from the job queue.
          \return the dequeued job.
        */
        JEThreadJob DequeueJob();

        //! Perform thread job.
        /*!
          Helper function for executing a job's function on its data.
          \param threadJob the job to execute.
        */
        void ThreadDoJob(JEThreadJob threadJob);

        //! Join threads.
        /*! Rejoin all threads in the pool to the main thread before terminating the application. */
        void JoinThreads();

        //! Stop thread pool.
        /*! Completely stop the threadpool, remove all jobs, and join all threads. */
        void StopThreadJobs();

    public:
        //! Constructor.
        /*! Creates and launches all threads in the pool. */
        JEThreadPool() : m_quit(false) {
            for (uint32_t t = 0; t < std::thread::hardware_concurrency(); ++t) { // TODO: one less than main thread?
                m_threads.emplace_back(std::thread([this] { ThreadFunction(); }));
            }
        }

        //! Destructor.
        /*! Joins threads. */
        ~JEThreadPool() {
            JoinThreads();
        }

        //! Enqueue thread job.
        /*!
          Synchronously add a new thread job to the job queue.
          \param job the job to enqueue.
        */
        void EnqueueJob(JEThreadJob job);
    };

    //! Thread pool instance.
    /*! The single thread pool instance. Note: extern, not static. */
    extern JEThreadPool JEThreadPoolInstance;
}
