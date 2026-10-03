#pragma once
#include "una_strength/free_workout.hpp"
#include "una_strength/controller.hpp"
#include <cstdint>

namespace una_strength {

enum class FreeScreen {
    SelectExercise,
    Set,
    EditWeight,
    EditReps,
    ConfirmSet,
    Rest,
    Actions,
    AssessmentQuality,
    AssessmentStrength,
    AssessmentEnergy,
    AssessmentRpe,
    AssessmentPain,
    AssessmentExpectation,
    Summary
};

class FreeWorkoutController {
public:
    FreeWorkoutController(const std::string& date, std::uint64_t start_ms);

    void press(Button button, std::uint64_t now_ms);
    ViewModel view(std::uint64_t now_ms) const;
    FreeScreen screen() const { return screen_; }
    const WorkoutResult* result() const { return result_ready_ ? &result_ : nullptr; }

private:
    FreeWorkoutSession session_;
    FreeScreen screen_{FreeScreen::SelectExercise};
    std::size_t library_index_{0};
    int action_index_{0};
    std::uint64_t set_started_ms_{0};
    SubjectiveAssessment assessment_{};
    int expectation_index_{1};
    bool result_ready_{false};
    WorkoutResult result_{};

    static std::string weightString(double value);
    static std::string durationString(std::uint64_t ms);
    void finishAssessment(std::uint64_t now_ms);
};

} // namespace una_strength
