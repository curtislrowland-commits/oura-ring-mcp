#pragma once
#include "model.hpp"
#include <cstddef>
#include <cstdint>
#include <string>

namespace una_strength {

class Session {
public:
    Session(const WorkoutPlan& plan, std::uint64_t start_ms);
    const WorkoutPlan& plan() const { return plan_; }
    std::size_t exercise_index() const { return ex_; }
    std::size_t set_index() const { return set_; }
    bool finished() const { return finished_; }
    int current_reps() const { return edit_reps_; }
    double current_weight() const { return edit_weight_; }
    void edit_reps(int reps);
    void edit_weight(double lb);
    void complete_set(std::uint64_t now_ms, std::uint64_t set_duration_ms, int auto_count=-1);
    void skip_current_set(std::uint64_t now_ms);
    void substitute_current_exercise(const std::string& new_id,const std::string& new_name);
    void skip_current_exercise(std::uint64_t now_ms);
    std::uint64_t elapsed_rest_ms(std::uint64_t now_ms) const;
    void end_rest(std::uint64_t now_ms);
    WorkoutResult finish(std::uint64_t end_ms, const SubjectiveAssessment& assessment);
private:
    WorkoutPlan plan_;
    WorkoutResult result_;
    std::size_t ex_{0},set_{0};
    int edit_reps_{0};
    double edit_weight_{0};
    bool finished_{false};
    std::uint64_t last_set_end_ms_{0};
    void load_current();
    void advance();
};
}
