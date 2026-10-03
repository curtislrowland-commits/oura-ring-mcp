#include "una_strength/rep_counter.hpp"
#include <cmath>
namespace una_strength {
bool RepCounter::profileFor(const std::string& id, RepProfile& out){
    if(id=="pendulum_squat"||id=="bulgarian_split_squat"){
        out={id,MotionSignal::AccelY,12.0f,10.2f,350,4500};return true;
    }
    if(id=="shoulder_press"){
        out={id,MotionSignal::AccelZ,12.0f,10.2f,300,4000};return true;
    }
    if(id=="bench_press"){
        out={id,MotionSignal::GyroYAbs,45.0f,15.0f,250,3500};return true;
    }
    return false;
}
float RepCounter::value(const MotionSample&s)const{
    switch(profile_.signal){
        case MotionSignal::AccelX:return s.ax;
        case MotionSignal::AccelY:return s.ay;
        case MotionSignal::AccelZ:return s.az;
        case MotionSignal::GyroXAbs:return std::fabs(s.gx);
        case MotionSignal::GyroYAbs:return std::fabs(s.gy);
        case MotionSignal::GyroZAbs:return std::fabs(s.gz);
    }
    return 0;
}
bool RepCounter::feed(const MotionSample&s){
    const float v=value(s);
    if(phase_==0){
        if(v>=profile_.high_threshold){phase_=1;peak_at_=s.timestamp_ms;}
        return false;
    }
    const auto elapsed=s.timestamp_ms>=peak_at_?s.timestamp_ms-peak_at_:0;
    if(elapsed>profile_.max_rep_ms){phase_=0;peak_at_=0;return false;}
    if(v<=profile_.low_threshold && elapsed>=profile_.min_rep_ms){
        ++count_;phase_=0;peak_at_=0;return true;
    }
    return false;
}
}
