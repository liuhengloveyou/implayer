#ifndef THREADBASE_H
#define THREADBASE_H

#ifdef __EMSCRIPTEN_PTHREADS__
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#else
#include <thread>
#include <memory>
#endif

class ThreadBase
{
public:
    virtual void threadMain() = 0;

protected:
    void stopThread();
    void startThread();

protected:
    bool m_thread_stop = false;

private:
#ifdef __EMSCRIPTEN_PTHREADS__
    pthread_t tid_;
#else
    std::unique_ptr<std::thread> thread_;
#endif
};

#endif