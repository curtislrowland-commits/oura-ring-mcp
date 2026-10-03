#include "una_strength/workout_repository.hpp"
#include "una_strength/json_io.hpp"
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>

namespace una_strength {

std::string WorkoutRepository::localDate()
{
    std::time_t utc = std::time(nullptr);
    std::tm local{};
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&local, &utc);
#else
    localtime_r(&utc, &local);
#endif
    char date[11]{};
    std::snprintf(date, sizeof(date), "%04d-%02d-%02d",
                  local.tm_year + 1900, local.tm_mon + 1, local.tm_mday);
    return date;
}

bool WorkoutRepository::loadScheduledForDate(SDK::Interface::IFileSystem& fs,
                                             const char* directory,
                                             const std::string& date,
                                             WorkoutPlan& out,
                                             std::string& selectedPath,
                                             std::string& error)
{
    auto dir = fs.dir(directory);
    if (!dir || !dir->exist() || !dir->open()) {
        error = "workout directory unavailable";
        return false;
    }

    bool found = false;
    SDK::Interface::IFileSystem::ObjectInfo item{};
    while (dir->readNext(item)) {
        if (item.isDir) continue;

        const char* dot = std::strrchr(item.name, '.');
        if (!dot || std::strcmp(dot, ".json") != 0) continue;

        std::string path = std::string(directory);
        if (!path.empty() && path.back() != '/') path += '/';
        path += item.name;

        WorkoutPlan candidate;
        std::string parseError;
        if (!JsonIO::loadWorkout(fs, path.c_str(), candidate, parseError)) {
            continue;
        }
        if (candidate.scheduled_date != date) {
            continue;
        }

        if (found) {
            dir->close();
            error = "multiple workouts scheduled for " + date;
            return false;
        }
        out = std::move(candidate);
        selectedPath = std::move(path);
        found = true;
    }
    dir->close();

    if (!found) {
        error = "no workout scheduled for " + date;
        return false;
    }
    return true;
}

} // namespace una_strength
