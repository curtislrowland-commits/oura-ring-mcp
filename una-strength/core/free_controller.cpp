#include "una_strength/free_controller.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace una_strength {

FreeWorkoutController::FreeWorkoutController(const std::string& date, std::uint64_t start_ms)
    : session_(date, start_ms), set_started_ms_(start_ms)
{}

std::string FreeWorkoutController::weightString(double value)
{
    std::ostringstream x;
    if (std::fabs(value - std::round(value)) < 0.001) x << static_cast<int>(std::round(value));
    else x << std::fixed << std::setprecision(1) << value;
    return x.str();
}

std::string FreeWorkoutController::durationString(std::uint64_t ms)
{
    auto sec = ms / 1000;
    std::ostringstream x;
    x << sec / 60 << ':' << std::setw(2) << std::setfill('0') << sec % 60;
    return x.str();
}

void FreeWorkoutController::press(Button b, std::uint64_t now_ms)
{
    switch (screen_) {
        case FreeScreen::SelectExercise: {
            const auto n = FreeWorkoutSession::library().size();
            if (b == Button::Up) library_index_ = (library_index_ + n - 1) % n;
            else if (b == Button::Down) library_index_ = (library_index_ + 1) % n;
            else if (b == Button::Select) {
                session_.addExercise(library_index_);
                set_started_ms_ = now_ms;
                screen_ = FreeScreen::Set;
            } else if (b == Button::Back && !session_.result().exercises.empty()) {
                screen_ = FreeScreen::Actions;
                action_index_ = 3;
            }
            break;
        }
        case FreeScreen::Set:
            if (b == Button::Up) screen_ = FreeScreen::EditWeight;
            else if (b == Button::Down) screen_ = FreeScreen::EditReps;
            else if (b == Button::Select) screen_ = FreeScreen::ConfirmSet;
            else if (b == Button::Back) { screen_ = FreeScreen::Actions; action_index_ = 0; }
            break;
        case FreeScreen::EditWeight:
            if (b == Button::Up) session_.setPendingWeight(session_.pendingWeight() + 5.0);
            else if (b == Button::Down) session_.setPendingWeight(std::max(0.0, session_.pendingWeight() - 5.0));
            else if (b == Button::Select || b == Button::Back) screen_ = FreeScreen::Set;
            break;
        case FreeScreen::EditReps:
            if (b == Button::Up) session_.setPendingReps(session_.pendingReps() + 1);
            else if (b == Button::Down) session_.setPendingReps(std::max(0, session_.pendingReps() - 1));
            else if (b == Button::Select || b == Button::Back) screen_ = FreeScreen::Set;
            break;
        case FreeScreen::ConfirmSet:
            if (b == Button::Select) {
                session_.completeSet(now_ms, now_ms >= set_started_ms_ ? now_ms - set_started_ms_ : 0);
                screen_ = FreeScreen::Rest;
            } else if (b == Button::Back) screen_ = FreeScreen::Set;
            break;
        case FreeScreen::Rest:
            if (b == Button::Select) {
                session_.endRest(now_ms);
                screen_ = FreeScreen::Actions;
                action_index_ = 0;
            }
            break;
        case FreeScreen::Actions:
            if (b == Button::Up) action_index_ = (action_index_ + 3) % 4;
            else if (b == Button::Down) action_index_ = (action_index_ + 1) % 4;
            else if (b == Button::Back) {
                set_started_ms_ = now_ms;
                screen_ = FreeScreen::Set;
            } else if (b == Button::Select) {
                if (action_index_ == 0) {
                    set_started_ms_ = now_ms;
                    screen_ = FreeScreen::Set;              // add another set
                } else if (action_index_ == 1) {
                    session_.removeLastSet();                // remove last completed set
                } else if (action_index_ == 2) {
                    screen_ = FreeScreen::SelectExercise;   // add another exercise
                } else {
                    screen_ = FreeScreen::AssessmentQuality;
                }
            }
            break;
        case FreeScreen::AssessmentQuality:
            if (b==Button::Up) assessment_.workout_quality=std::min(5,assessment_.workout_quality+1);
            else if(b==Button::Down) assessment_.workout_quality=std::max(1,assessment_.workout_quality-1);
            else if(b==Button::Select) screen_=FreeScreen::AssessmentStrength;
            break;
        case FreeScreen::AssessmentStrength:
            if(b==Button::Up) assessment_.strength=std::min(5,assessment_.strength+1);
            else if(b==Button::Down) assessment_.strength=std::max(1,assessment_.strength-1);
            else if(b==Button::Select) screen_=FreeScreen::AssessmentEnergy;
            else if(b==Button::Back) screen_=FreeScreen::AssessmentQuality;
            break;
        case FreeScreen::AssessmentEnergy:
            if(b==Button::Up) assessment_.energy=std::min(5,assessment_.energy+1);
            else if(b==Button::Down) assessment_.energy=std::max(1,assessment_.energy-1);
            else if(b==Button::Select) screen_=FreeScreen::AssessmentRpe;
            else if(b==Button::Back) screen_=FreeScreen::AssessmentStrength;
            break;
        case FreeScreen::AssessmentRpe:
            if(b==Button::Up) assessment_.session_rpe=std::min(10,assessment_.session_rpe+1);
            else if(b==Button::Down) assessment_.session_rpe=std::max(1,assessment_.session_rpe-1);
            else if(b==Button::Select) screen_=FreeScreen::AssessmentPain;
            else if(b==Button::Back) screen_=FreeScreen::AssessmentEnergy;
            break;
        case FreeScreen::AssessmentPain:
            if(b==Button::Up) assessment_.pain=std::min(10,assessment_.pain+1);
            else if(b==Button::Down) assessment_.pain=std::max(0,assessment_.pain-1);
            else if(b==Button::Select) screen_=FreeScreen::AssessmentExpectation;
            else if(b==Button::Back) screen_=FreeScreen::AssessmentRpe;
            break;
        case FreeScreen::AssessmentExpectation:
            if(b==Button::Up) expectation_index_=std::max(0,expectation_index_-1);
            else if(b==Button::Down) expectation_index_=std::min(2,expectation_index_+1);
            else if(b==Button::Select) finishAssessment(now_ms);
            else if(b==Button::Back) screen_=FreeScreen::AssessmentPain;
            break;
        case FreeScreen::Summary:
            break;
    }
}

void FreeWorkoutController::finishAssessment(std::uint64_t now_ms)
{
    static const char* values[]={"below_expected","as_expected","above_expected"};
    assessment_.expectation=values[expectation_index_];
    result_=session_.finish(now_ms,assessment_);
    result_ready_=true;
    screen_=FreeScreen::Summary;
}

ViewModel FreeWorkoutController::view(std::uint64_t now_ms) const
{
    ViewModel v;
    const auto& lib=FreeWorkoutSession::library();
    switch(screen_) {
        case FreeScreen::SelectExercise:
            v.title="Free Workout";
            v.lines={"Choose exercise",lib[library_index_].name};
            v.footer="L1/L2 choose  R1 add";
            break;
        case FreeScreen::Set:
            v.title=session_.currentExercise().actual_name;
            v.lines={"Set "+std::to_string(session_.completedSetsCurrent()+1),
                     "Weight: "+weightString(session_.pendingWeight())+" lb",
                     "Reps: "+std::to_string(session_.pendingReps())};
            v.footer="L1 wt L2 reps R1 done R2 menu";
            break;
        case FreeScreen::EditWeight:
            v.title="Edit weight";v.lines={weightString(session_.pendingWeight())+" lb"};v.footer="L1 +5 L2 -5 R1 save";break;
        case FreeScreen::EditReps:
            v.title="Edit reps";v.lines={std::to_string(session_.pendingReps())};v.footer="L1 +1 L2 -1 R1 save";break;
        case FreeScreen::ConfirmSet:
            v.title="Complete set?";v.lines={session_.currentExercise().actual_name,weightString(session_.pendingWeight())+" lb x "+std::to_string(session_.pendingReps())};v.footer="R1 confirm R2 back";break;
        case FreeScreen::Rest:
            v.title="REST";v.lines={"Elapsed "+durationString(session_.elapsedRestMs(now_ms)),"No countdown"};v.footer="R1 options";break;
        case FreeScreen::Actions: {
            static const char* actions[]={"Add set","Remove last set","Add exercise","Finish workout"};
            v.title="Free Workout";v.lines={actions[action_index_],"Completed sets: "+std::to_string(session_.completedSetsCurrent())};v.footer="L1/L2 choose R1 select R2 set";break;
        }
        case FreeScreen::AssessmentQuality:v.title="Workout quality";v.lines={std::to_string(assessment_.workout_quality)+" / 5"};v.footer="L1/L2 adjust R1 next";break;
        case FreeScreen::AssessmentStrength:v.title="Strength";v.lines={std::to_string(assessment_.strength)+" / 5"};v.footer="L1/L2 adjust R1 next";break;
        case FreeScreen::AssessmentEnergy:v.title="Energy";v.lines={std::to_string(assessment_.energy)+" / 5"};v.footer="L1/L2 adjust R1 next";break;
        case FreeScreen::AssessmentRpe:v.title="Session RPE";v.lines={std::to_string(assessment_.session_rpe)+" / 10"};v.footer="L1/L2 adjust R1 next";break;
        case FreeScreen::AssessmentPain:v.title="Pain/discomfort";v.lines={std::to_string(assessment_.pain)+" / 10"};v.footer="L1/L2 adjust R1 next";break;
        case FreeScreen::AssessmentExpectation:{static const char* labels[]={"Below expected","As expected","Above expected"};v.title="Performance";v.lines={labels[expectation_index_]};v.footer="L1/L2 choose R1 finish";break;}
        case FreeScreen::Summary:
            v.title="Saved";v.lines={"Reps: "+std::to_string(result_.total_reps),"Volume: "+weightString(result_.training_volume_lb)+" lb"};v.footer="Free workout complete";break;
    }
    return v;
}

} // namespace una_strength
