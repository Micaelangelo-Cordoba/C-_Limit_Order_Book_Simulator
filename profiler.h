#pragma once
#include <chrono>


class profiler {
    public:

    double timeElapsed();

    //time is measured in the difference in time_point snapshots.
    //this ensures that the starting point resets to the most recent time
    void reset() {m_time = Clock::now();}
private:

using Clock = std::chrono::steady_clock;

//std::chrono measures time in ticks. the 1 over 1 million ratio represents
//how much time every tick occurs with respect to time, so it takes 1 millionth of a second to occur
//mapping out to 1 microsecond. 
using microSeconds = std::chrono::duration<double, std::ratio<1,1000000>>;

//time is measured in the difference in ticks of each snapshot
//recorded from the steady_clock
using TimePoint_snap = std::chrono::time_point<Clock>;

TimePoint_snap m_time {Clock::now()};
//clock::now() marks a snapshot as a std::chrono::time_point<Clock>


};

inline double profiler::timeElapsed() {
    return std::chrono::duration_cast<microSeconds>(Clock::now() - m_time).count();
}