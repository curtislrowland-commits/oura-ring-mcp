#include "una_strength/heart_rate.hpp"
#include <algorithm>
#include <cmath>

namespace una_strength {

void HeartRateTracker::add(std::uint64_t timestamp_ms, float bpm, float trust_level)
{
    // UNA documents trust level 0 as no signal. Keep only plausible, trusted
    // workout readings; raw optical accuracy remains a hardware validation item.
    if (trust_level <= 0.0f || bpm < 20.0f || bpm > 250.0f) return;
    HeartRateSample s;
    s.timestamp_ms = timestamp_ms;
    s.bpm = static_cast<int>(std::lround(bpm));
    s.trust_level = trust_level;
    samples_.push_back(s);
}

HeartRateSummary HeartRateTracker::summary() const
{
    HeartRateSummary out;
    out.samples = samples_;
    out.sample_count = static_cast<int>(samples_.size());
    if (samples_.empty()) return out;

    out.available = true;
    out.min_bpm = samples_.front().bpm;
    out.max_bpm = samples_.front().bpm;
    long long total = 0;
    for (const auto& s : samples_) {
        out.min_bpm = std::min(out.min_bpm, s.bpm);
        out.max_bpm = std::max(out.max_bpm, s.bpm);
        total += s.bpm;
    }
    out.avg_bpm = static_cast<double>(total) / samples_.size();
    return out;
}

} // namespace una_strength
