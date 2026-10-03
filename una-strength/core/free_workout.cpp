#include "una_strength/free_workout.hpp"
#include <algorithm>

namespace una_strength {

const std::vector<ExerciseDefinition>& FreeWorkoutSession::library()
{
    static const std::vector<ExerciseDefinition> items{
        {"pendulum_squat", "Pendulum Squat", 55.0, 5},
        {"bench_press", "Bench Press", 185.0, 5},
        {"shoulder_press", "Shoulder Press", 75.0, 5},
        {"seated_leg_curl", "Seated Leg Curl", 80.0, 10}
    };
    return items;
}

FreeWorkoutSession::FreeWorkoutSession(const std::string& date, std::uint64_t start_ms)
{
    result_.workout_id = "free-" + date;
    result_.workout_name = "Free Workout";
    result_.date = date;
    result_.started_at_unix_ms = start_ms;
}

void FreeWorkoutSession::loadDefaults(const ExerciseDefinition& def)
{
    pending_weight_ = def.default_weight_lb;
    pending_reps_ = def.default_reps;
}

bool FreeWorkoutSession::addExercise(std::size_t libraryIndex)
{
    const auto& lib = library();
    if (libraryIndex >= lib.size()) return false;
    const auto& def = lib[libraryIndex];

    CompletedExercise ex;
    ex.id = def.id;
    ex.prescribed_name = "";
    ex.actual_name = def.name;
    ex.prescribed_order = 0;
    ex.actual_order = static_cast<int>(result_.exercises.size() + 1);
    ex.added = true;
    result_.exercises.push_back(ex);
    current_exercise_ = result_.exercises.size() - 1;
    loadDefaults(def);
    return true;
}

void FreeWorkoutSession::setPendingReps(int reps)
{
    pending_reps_ = std::max(0, std::min(1000, reps));
}

void FreeWorkoutSession::setPendingWeight(double weight)
{
    pending_weight_ = std::max(0.0, std::min(5000.0, weight));
}

void FreeWorkoutSession::completeSet(std::uint64_t now_ms, std::uint64_t set_duration_ms)
{
    if (!hasExercise()) return;
    CompletedSet s;
    s.set_number = static_cast<int>(result_.exercises[current_exercise_].sets.size() + 1);
    s.prescribed_reps = 0;
    s.prescribed_weight_lb = 0.0;
    s.actual_reps = pending_reps_;
    s.actual_weight_lb = pending_weight_;
    s.set_duration_ms = set_duration_ms;
    result_.exercises[current_exercise_].sets.push_back(s);
    result_.total_reps += s.actual_reps;
    result_.training_volume_lb += s.actual_reps * s.actual_weight_lb;
    last_set_end_ms_ = now_ms;
}

bool FreeWorkoutSession::removeLastSet()
{
    if (!hasExercise()) return false;
    auto& sets = result_.exercises[current_exercise_].sets;
    if (sets.empty()) return false;
    sets.pop_back();
    recalcTotals();
    return true;
}

std::size_t FreeWorkoutSession::completedSetsCurrent() const
{
    return hasExercise() ? result_.exercises[current_exercise_].sets.size() : 0;
}

std::uint64_t FreeWorkoutSession::elapsedRestMs(std::uint64_t now_ms) const
{
    return last_set_end_ms_ ? now_ms - last_set_end_ms_ : 0;
}

void FreeWorkoutSession::endRest(std::uint64_t now_ms)
{
    if (!last_set_end_ms_ || !hasExercise()) return;
    auto& sets = result_.exercises[current_exercise_].sets;
    if (!sets.empty()) sets.back().rest_duration_ms = now_ms - last_set_end_ms_;
}

void FreeWorkoutSession::recalcTotals()
{
    result_.total_reps = 0;
    result_.training_volume_lb = 0.0;
    for (const auto& ex : result_.exercises) {
        for (const auto& s : ex.sets) {
            if (!s.skipped) {
                result_.total_reps += s.actual_reps;
                result_.training_volume_lb += s.actual_reps * s.actual_weight_lb;
            }
        }
    }
}

WorkoutResult FreeWorkoutSession::finish(std::uint64_t end_ms, const SubjectiveAssessment& assessment)
{
    if (result_.exercises.empty()) return result_;
    result_.ended_at_unix_ms = end_ms;
    result_.duration_ms = end_ms - result_.started_at_unix_ms;
    result_.assessment = assessment;
    recalcTotals();
    return result_;
}

} // namespace una_strength
