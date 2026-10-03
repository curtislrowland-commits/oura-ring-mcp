#pragma once
#include <cstdint>
#include <vector>

namespace una_strength {

struct HeartRateSample {
    std::uint64_t timestamp_ms{0};
    int bpm{0};
    float trust_level{0.0f};
};

struct HeartRateSummary {
    bool available{false};
    int sample_count{0};
    int min_bpm{0};
    int max_bpm{0};
    double avg_bpm{0.0};
    std::vector<HeartRateSample> samples;
};

class HeartRateTracker {
public:
    void add(std::uint64_t timestamp_ms, float bpm, float trust_level);
    HeartRateSummary summary() const;
private:
    std::vector<HeartRateSample> samples_;
};

} // namespace una_strength
