#pragma once
#include <cstdint>
#include <string>
namespace una_strength {
struct MotionSample {
    std::uint64_t timestamp_ms{0};
    float ax{0},ay{0},az{0};
    float gx{0},gy{0},gz{0};
};
enum class MotionSignal { AccelX, AccelY, AccelZ, GyroXAbs, GyroYAbs, GyroZAbs };
struct RepProfile {
    std::string exercise_id;
    MotionSignal signal{MotionSignal::AccelY};
    float high_threshold{12.0f};
    float low_threshold{10.0f};
    std::uint64_t min_rep_ms{300};
    std::uint64_t max_rep_ms{4000};
};
class RepCounter {
public:
    explicit RepCounter(RepProfile profile):profile_(profile){}
    static bool profileFor(const std::string& exercise_id, RepProfile& out);
    bool feed(const MotionSample& sample);
    int count() const{return count_;}
    void reset(){phase_=0;count_=0;peak_at_=0;}
private:
    RepProfile profile_;
    int phase_{0};
    int count_{0};
    std::uint64_t peak_at_{0};
    float value(const MotionSample& s) const;
};
}
