#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <optional>

namespace una_strength {

struct SetPrescription {
    int target_reps{0};
    double target_weight_lb{0.0};
};

struct ExercisePlan {
    std::string id;
    std::string name;
    std::vector<SetPrescription> sets;
};

struct WorkoutPlan {
    int schema_version{1};
    std::string workout_id;
    std::string name;
    std::string scheduled_date;
    std::vector<ExercisePlan> exercises;
};

struct CompletedSet {
    int set_number{0};
    int prescribed_reps{0};
    double prescribed_weight_lb{0.0};
    int actual_reps{0};
    double actual_weight_lb{0.0};
    std::uint64_t set_duration_ms{0};
    std::uint64_t rest_duration_ms{0};
    bool skipped{false};
    bool auto_rep_count_used{false};
    int auto_rep_count{0};
    bool rep_count_corrected{false};
};

struct CompletedExercise {
    std::string id;
    std::string prescribed_name;
    std::string actual_name;
    int prescribed_order{0};
    int actual_order{0};
    bool skipped{false};
    bool substituted{false};
    bool added{false};
    std::vector<CompletedSet> sets;
};

struct SubjectiveAssessment {
    int workout_quality{3};
    int strength{3};
    int energy{3};
    int session_rpe{5};
    int pain{0};
    std::string expectation{"as_expected"};
    std::vector<std::string> tags;
    std::string notes;
};

struct WorkoutResult {
    int schema_version{1};
    std::string workout_id;
    std::string workout_name;
    std::string date;
    std::uint64_t started_at_unix_ms{0};
    std::uint64_t ended_at_unix_ms{0};
    std::uint64_t duration_ms{0};
    int total_reps{0};
    double training_volume_lb{0.0};
    std::vector<CompletedExercise> exercises;
    SubjectiveAssessment assessment;
};

} // namespace una_strength
