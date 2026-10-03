#pragma once
#include "una_strength/model.hpp"
#include "SDK/Interfaces/IFileSystem.hpp"
#include <string>

namespace una_strength {
class JsonIO {
public:
    static bool loadWorkout(SDK::Interface::IFileSystem& fs, const char* path,
                            WorkoutPlan& out, std::string& error);
    static bool saveResult(SDK::Interface::IFileSystem& fs, const char* path,
                           const WorkoutResult& result, std::string& error);
};
}
