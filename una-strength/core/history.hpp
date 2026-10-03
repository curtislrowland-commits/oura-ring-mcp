#pragma once
#include "SDK/Interfaces/IFileSystem.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace una_strength {

struct HistoryEntry {
    std::string path;
    std::string workout_id;
    std::string workout_name;
    std::string date;
    std::uint64_t started_at_unix_ms{0};
    std::uint64_t ended_at_unix_ms{0};
    std::uint64_t duration_ms{0};
    int total_reps{0};
    double training_volume_lb{0.0};
    int workout_quality{0};
    int session_rpe{0};
    int pain{0};
    std::string expectation;
    int hr_sample_count{0};
    int hr_min_bpm{0};
    int hr_max_bpm{0};
    double hr_avg_bpm{0.0};
};

class WorkoutHistory {
public:
    static bool load(SDK::Interface::IFileSystem& fs,
                     const char* directory,
                     std::vector<HistoryEntry>& out,
                     std::string& error);
private:
    static bool readEntry(SDK::Interface::IFileSystem& fs,
                          const char* path,
                          HistoryEntry& out);
};

} // namespace una_strength
