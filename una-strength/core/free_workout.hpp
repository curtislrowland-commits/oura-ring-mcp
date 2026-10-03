#pragma once
#include "una_strength/model.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace una_strength {

struct ExerciseDefinition {
    const char* id;
    const char* name;
    double default_weight_lb;
    int default_reps;
};

class FreeWorkoutSession {
public:
    explicit FreeWorkoutSession(const std::string& date, std::uint64_t start_ms);

    static const std::vector<ExerciseDefinition>& library();

    bool addExercise(std::size_t libraryIndex);
    bool hasExercise() const { return current_exercise_ < result_.exercises.size(); }
    std::size_t currentExerciseIndex() const { return current_exercise_; }
    const CompletedExercise& currentExercise() const { return result_.exercises.at(current_exercise_); }
    const WorkoutResult& result() const { return result_; }

    int pendingReps() const { return pending_reps_; }
    double pendingWeight() const { return pending_weight_; }
    void setPendingReps(int reps);
    void setPendingWeight(double weight);

    void completeSet(std::uint64_t now_ms, std::uint64_t set_duration_ms);
    bool removeLastSet();
    std::size_t completedSetsCurrent() const;

    std::uint64_t elapsedRestMs(std::uint64_t now_ms) const;
    void endRest(std::uint64_t now_ms);

    WorkoutResult finish(std::uint64_t end_ms, const SubjectiveAssessment& assessment);

private:
    WorkoutResult result_;
    std::size_t current_exercise_{0};
    int pending_reps_{5};
    double pending_weight_{0.0};
    std::uint64_t last_set_end_ms_{0};
    void loadDefaults(const ExerciseDefinition& def);
    void recalcTotals();
};

} // namespace una_strength
