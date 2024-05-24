#ifndef IMPLAYER_TIME_UTILS_H
#define IMPLAYER_TIME_UTILS_H

#include <chrono>

long long getTimestamp()
{
    auto millisec_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    //  auto sec_since_epoch = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    return millisec_since_epoch;
}

#endif