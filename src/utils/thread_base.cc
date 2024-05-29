#include "thread_base.h"
#include <stdio.h>

static void *ThreadEntry(void *arg)
{
    ThreadBase *th = (ThreadBase *)arg;
    th->threadMain();
    return NULL;
}

void ThreadBase::stopThread()
{
    m_thread_stop = true;

#ifdef __EMSCRIPTEN_PTHREADS__
    // pthread_join(tid_, NULL);
    tid_ = 0;
#else
    thread_->join();
    thread_ = nullptr;
#endif
}

void ThreadBase::startThread()
{
    m_thread_stop = false;

#ifdef __EMSCRIPTEN_PTHREADS__
    pthread_create(&tid_, NULL, ThreadEntry, this);
    pthread_detach(tid_);
#else
    thread_ = std::make_unique<std::thread>(ThreadEntry, this);
#endif
}