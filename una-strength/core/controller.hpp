#pragma once
#include "session.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace una_strength {
enum class Button { Up, Down, Select, Back };
enum class Screen { Workout, EditWeight, EditReps, ConfirmSet, Rest, AssessmentQuality, AssessmentStrength, AssessmentEnergy, AssessmentRpe, AssessmentPain, AssessmentExpectation, Summary };
struct ViewModel { Screen screen{Screen::Workout}; std::string title; std::vector<std::string> lines; std::string footer; };
class WorkoutController {
public:
    WorkoutController(const WorkoutPlan& plan, std::uint64_t start_ms);
    void press(Button button, std::uint64_t now_ms);
    ViewModel view(std::uint64_t now_ms) const;
    bool done() const { return screen_ == Screen::Summary; }
    const WorkoutResult* result() const { return result_ready_ ? &result_ : nullptr; }
private:
    Session session_;
    Screen screen_{Screen::Workout};
    std::uint64_t set_started_ms_{0};
    std::uint64_t last_now_ms_{0};
    bool result_ready_{false};
    WorkoutResult result_{};
    SubjectiveAssessment assessment_{};
    int expectation_index_{1};
    static std::string weight_string(double value);
    static std::string duration_string(std::uint64_t ms);
    void finish_assessment(std::uint64_t now_ms);
};
}
