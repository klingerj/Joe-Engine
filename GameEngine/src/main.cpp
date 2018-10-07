#include <stdio.h>
#include <iostream>
#include <string>
#include <future>
#include <thread>
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

void DoStuff(void* data) {

}

struct sample_data_t {
    std::string name;
    int id;
};

void ManipulateData(void* data) {
    sample_data_t* sampleData = static_cast<sample_data_t*>(data);
    sampleData->name = "Work done";
    sampleData->id++;
}

struct thread_job_t {
    void(*ManipulateData)(void*);
    void* data;
};

void ThreadDoJob(const thread_job_t& threadJob) {
    threadJob.ManipulateData(threadJob.data);
}

int main() {
    //return RunApp();

    // Multithreading testing

    // http://fabiensanglard.net/doom3_bfg/threading.php
    sample_data_t data = { "Work not done yet", -1 };
    void* dataPtr = &data;
    thread_job_t job = { &ManipulateData, dataPtr };
    
    std::future<void> f = std::async(std::launch::async, ThreadDoJob, job);


    /*
    // future from a packaged_task
    std::packaged_task<void()> task([] { return 7; }); // wrap the function
    std::future<void> f1 = task.get_future();  // get a future
    std::thread t(std::move(task)); // launch on a thread

    // future from an async()
    std::future<int> f2 = std::async(std::launch::async, [] { return 8; });

    // future from a promise
    std::promise<int> p;
    std::future<int> f3 = p.get_future();
    std::thread([&p] { p.set_value_at_thread_exit(9); }).detach();

    std::cout << "Waiting..." << std::flush;
    f1.wait();
    f2.wait();
    f3.wait();
    std::cout << "Done!\nResults are: "
        << f2.get() << ' ' << f3.get() << '\n';
    t.join();*/

    f.wait();
    std::cout << "Result: " << data.name << ", " << data.id << std::endl;

    return EXIT_SUCCESS;
}
