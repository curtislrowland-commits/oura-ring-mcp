#pragma once
#include "una_strength/model.hpp"
#include "SDK/Interfaces/IFileSystem.hpp"
#include <string>

namespace una_strength {

class WorkoutRepository {
public:
    // Returns local watch date in YYYY-MM-DD using the same libc clock available
    // to UNA apps. The simulator and watch both use time()/localtime_r().
    static std::string localDate();

    // Scans a directory of JSON workout files and selects the single workout
    // whose scheduled_date equals date. Invalid/non-JSON files are ignored.
    // Multiple workouts for the same date are rejected to avoid ambiguity.
    static bool loadScheduledForDate(SDK::Interface::IFileSystem& fs,
                                     const char* directory,
                                     const std::string& date,
                                     WorkoutPlan& out,
                                     std::string& selectedPath,
                                     std::string& error);
};

} // namespace una_strength
