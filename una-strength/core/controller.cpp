#include "una_strength/controller.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>
namespace una_strength {
const std::vector<ExercisePlan>& WorkoutController::exercise_library(){
    static const std::vector<ExercisePlan> lib={
        {"pendulum_squat","Pendulum Squat",{{5,55.0}}},
        {"bench_press","Bench Press",{{5,185.0}}},
        {"shoulder_press","Shoulder Press",{{5,75.0}}},
        {"seated_leg_curl","Seated Leg Curl",{{10,80.0}}},
        {"bulgarian_split_squat","Bulgarian Split Squat",{{8,30.0}}}
    };
    return lib;
}
WorkoutController::WorkoutController(const WorkoutPlan& plan, std::uint64_t start_ms):session_(plan,start_ms),set_started_ms_(start_ms),last_now_ms_(start_ms){ if(session_.finished()) throw std::runtime_error("workout has no sets"); }
std::string WorkoutController::weight_string(double value){ std::ostringstream x; if(std::fabs(value-std::round(value))<0.001)x<<static_cast<int>(std::round(value)); else x<<std::fixed<<std::setprecision(1)<<value; return x.str(); }
std::string WorkoutController::duration_string(std::uint64_t ms){ auto total=ms/1000; auto min=total/60; auto sec=total%60; std::ostringstream x; x<<min<<':'<<std::setw(2)<<std::setfill('0')<<sec; return x.str(); }
void WorkoutController::press(Button b,std::uint64_t now_ms){ last_now_ms_=now_ms; switch(screen_){
case Screen::Workout:
    if(b==Button::Up)screen_=Screen::EditWeight;
    else if(b==Button::Down)screen_=Screen::EditReps;
    else if(b==Button::Select)screen_=Screen::ConfirmSet;
    else if(b==Button::Back){screen_=Screen::Actions;action_index_=0;}
    break;
case Screen::EditWeight: if(b==Button::Up)session_.edit_weight(session_.current_weight()+5.0); else if(b==Button::Down)session_.edit_weight(std::max(0.0,session_.current_weight()-5.0)); else if(b==Button::Select||b==Button::Back)screen_=Screen::Workout; break;
case Screen::EditReps: if(b==Button::Up)session_.edit_reps(session_.current_reps()+1); else if(b==Button::Down)session_.edit_reps(std::max(0,session_.current_reps()-1)); else if(b==Button::Select||b==Button::Back)screen_=Screen::Workout; break;
case Screen::ConfirmSet: if(b==Button::Select){ auto dur=now_ms>=set_started_ms_?now_ms-set_started_ms_:0; session_.complete_set(now_ms,dur); if(session_.finished())screen_=Screen::AssessmentQuality; else screen_=Screen::Rest; } else if(b==Button::Back)screen_=Screen::Workout; break;
case Screen::Rest:
    if(b==Button::Select){session_.end_rest(now_ms);set_started_ms_=now_ms;screen_=Screen::Workout;}
    else if(b==Button::Back){screen_=Screen::Actions;action_index_=0;}
    break;
case Screen::Actions:
    if(b==Button::Up) action_index_=(action_index_+6)%7;
    else if(b==Button::Down) action_index_=(action_index_+1)%7;
    else if(b==Button::Back) screen_=Screen::Workout;
    else if(b==Button::Select){
        if(action_index_==0){session_.skip_current_set(now_ms);screen_=session_.finished()?Screen::AssessmentQuality:Screen::Workout;}
        else if(action_index_==1){session_.skip_current_exercise(now_ms);screen_=session_.finished()?Screen::AssessmentQuality:Screen::Workout;}
        else if(action_index_==2){session_.add_set_current();screen_=Screen::Workout;}
        else if(action_index_==3){session_.remove_current_set();screen_=Screen::Workout;}
        else if(action_index_==4){exercise_choice_=0;screen_=Screen::ChooseSubstitute;}
        else if(action_index_==5){exercise_choice_=0;screen_=Screen::ChooseAddExercise;}
        else {session_.move_current_exercise_later();screen_=Screen::Workout;}
    }
    break;
case Screen::ChooseSubstitute: {
    const auto n=exercise_library().size();
    if(b==Button::Up) exercise_choice_=(exercise_choice_+n-1)%n;
    else if(b==Button::Down) exercise_choice_=(exercise_choice_+1)%n;
    else if(b==Button::Back) screen_=Screen::Actions;
    else if(b==Button::Select){const auto&e=exercise_library()[exercise_choice_];session_.substitute_current_exercise(e.id,e.name);screen_=Screen::Workout;}
    break;
}
case Screen::ChooseAddExercise: {
    const auto n=exercise_library().size();
    if(b==Button::Up) exercise_choice_=(exercise_choice_+n-1)%n;
    else if(b==Button::Down) exercise_choice_=(exercise_choice_+1)%n;
    else if(b==Button::Back) screen_=Screen::Actions;
    else if(b==Button::Select){const auto&e=exercise_library()[exercise_choice_];session_.add_exercise(e.id,e.name,e.sets[0].target_weight_lb,e.sets[0].target_reps);screen_=Screen::Workout;}
    break;
}
case Screen::AssessmentQuality: if(b==Button::Up)assessment_.workout_quality=std::min(5,assessment_.workout_quality+1); else if(b==Button::Down)assessment_.workout_quality=std::max(1,assessment_.workout_quality-1); else if(b==Button::Select)screen_=Screen::AssessmentStrength; break;
case Screen::AssessmentStrength: if(b==Button::Up)assessment_.strength=std::min(5,assessment_.strength+1); else if(b==Button::Down)assessment_.strength=std::max(1,assessment_.strength-1); else if(b==Button::Select)screen_=Screen::AssessmentEnergy; else if(b==Button::Back)screen_=Screen::AssessmentQuality; break;
case Screen::AssessmentEnergy: if(b==Button::Up)assessment_.energy=std::min(5,assessment_.energy+1); else if(b==Button::Down)assessment_.energy=std::max(1,assessment_.energy-1); else if(b==Button::Select)screen_=Screen::AssessmentRpe; else if(b==Button::Back)screen_=Screen::AssessmentStrength; break;
case Screen::AssessmentRpe: if(b==Button::Up)assessment_.session_rpe=std::min(10,assessment_.session_rpe+1); else if(b==Button::Down)assessment_.session_rpe=std::max(1,assessment_.session_rpe-1); else if(b==Button::Select)screen_=Screen::AssessmentPain; else if(b==Button::Back)screen_=Screen::AssessmentEnergy; break;
case Screen::AssessmentPain: if(b==Button::Up)assessment_.pain=std::min(10,assessment_.pain+1); else if(b==Button::Down)assessment_.pain=std::max(0,assessment_.pain-1); else if(b==Button::Select)screen_=Screen::AssessmentExpectation; else if(b==Button::Back)screen_=Screen::AssessmentRpe; break;
case Screen::AssessmentExpectation: if(b==Button::Up)expectation_index_=std::max(0,expectation_index_-1); else if(b==Button::Down)expectation_index_=std::min(2,expectation_index_+1); else if(b==Button::Select)finish_assessment(now_ms); else if(b==Button::Back)screen_=Screen::AssessmentPain; break;
case Screen::Summary: break; } }
void WorkoutController::finish_assessment(std::uint64_t now_ms){ static const char* k[]={"below_expected","as_expected","above_expected"}; assessment_.expectation=k[expectation_index_]; result_=session_.finish(now_ms,assessment_); result_ready_=true; screen_=Screen::Summary; }
ViewModel WorkoutController::view(std::uint64_t now_ms)const{ ViewModel v;v.screen=screen_; const auto&p=session_.plan(); const std::string ex=!session_.finished()?p.exercises.at(session_.exercise_index()).name:"Workout complete"; switch(screen_){
case Screen::Workout:v.title=ex;v.lines={"Set "+std::to_string(session_.set_index()+1)+"/"+std::to_string(p.exercises.at(session_.exercise_index()).sets.size()),"Weight: "+weight_string(session_.current_weight())+" lb","Reps: "+std::to_string(session_.current_reps())};v.footer="L1 wt L2 reps R1 done R2 menu";break;
case Screen::EditWeight:v.title="Edit weight";v.lines={weight_string(session_.current_weight())+" lb"};v.footer="L1 +5  L2 -5  R1 save";break;
case Screen::EditReps:v.title="Edit reps";v.lines={std::to_string(session_.current_reps())};v.footer="L1 +1  L2 -1  R1 save";break;
case Screen::ConfirmSet:v.title="Complete set?";v.lines={ex,weight_string(session_.current_weight())+" lb x "+std::to_string(session_.current_reps())};v.footer="R1 confirm  R2 back";break;
case Screen::Rest:v.title="REST";v.lines={"Elapsed "+duration_string(session_.elapsed_rest_ms(now_ms)),"No countdown"};v.footer="R1 next set R2 menu";break;
case Screen::Actions:{static const char* a[]={"Skip set","Skip exercise","Add set","Remove set","Substitute","Add exercise","Move exercise later"};v.title="Workout options";v.lines={a[action_index_]};v.footer="L1/L2 choose R1 select R2 back";break;}
case Screen::ChooseSubstitute:v.title="Substitute";v.lines={exercise_library()[exercise_choice_].name};v.footer="L1/L2 choose R1 use R2 back";break;
case Screen::ChooseAddExercise:v.title="Add exercise";v.lines={exercise_library()[exercise_choice_].name};v.footer="L1/L2 choose R1 add R2 back";break;
case Screen::AssessmentQuality:v.title="Workout quality";v.lines={std::to_string(assessment_.workout_quality)+" / 5"};v.footer="L1/L2 adjust  R1 next";break;
case Screen::AssessmentStrength:v.title="Strength";v.lines={std::to_string(assessment_.strength)+" / 5"};v.footer="L1/L2 adjust  R1 next";break;
case Screen::AssessmentEnergy:v.title="Energy";v.lines={std::to_string(assessment_.energy)+" / 5"};v.footer="L1/L2 adjust  R1 next";break;
case Screen::AssessmentRpe:v.title="Session RPE";v.lines={std::to_string(assessment_.session_rpe)+" / 10"};v.footer="L1/L2 adjust  R1 next";break;
case Screen::AssessmentPain:v.title="Pain/discomfort";v.lines={std::to_string(assessment_.pain)+" / 10"};v.footer="L1/L2 adjust  R1 next";break;
case Screen::AssessmentExpectation:{static const char* labels[]={"Below expected","As expected","Above expected"};v.title="Performance";v.lines={labels[expectation_index_]};v.footer="L1/L2 choose  R1 finish";break;}
case Screen::Summary:v.title="Saved";v.lines={"Reps: "+std::to_string(result_.total_reps),"Volume: "+weight_string(result_.training_volume_lb)+" lb"};v.footer="Workout complete";break;} return v; }
}
