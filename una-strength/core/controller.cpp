#include "controller.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>
namespace una_strength {
WorkoutController::WorkoutController(const WorkoutPlan& plan, std::uint64_t start_ms):session_(plan,start_ms),set_started_ms_(start_ms),last_now_ms_(start_ms){ if(session_.finished()) throw std::runtime_error("workout has no sets"); }
std::string WorkoutController::weight_string(double value){ std::ostringstream x; if(std::fabs(value-std::round(value))<0.001)x<<static_cast<int>(std::round(value)); else x<<std::fixed<<std::setprecision(1)<<value; return x.str(); }
std::string WorkoutController::duration_string(std::uint64_t ms){ auto total=ms/1000; auto min=total/60; auto sec=total%60; std::ostringstream x; x<<min<<':'<<std::setw(2)<<std::setfill('0')<<sec; return x.str(); }
void WorkoutController::press(Button b,std::uint64_t now_ms){ last_now_ms_=now_ms; switch(screen_){
case Screen::Workout: if(b==Button::Up)screen_=Screen::EditWeight; else if(b==Button::Down)screen_=Screen::EditReps; else if(b==Button::Select)screen_=Screen::ConfirmSet; else if(b==Button::Back)session_.skip_current_set(now_ms); if(session_.finished())screen_=Screen::AssessmentQuality; break;
case Screen::EditWeight: if(b==Button::Up)session_.edit_weight(session_.current_weight()+5.0); else if(b==Button::Down)session_.edit_weight(std::max(0.0,session_.current_weight()-5.0)); else if(b==Button::Select||b==Button::Back)screen_=Screen::Workout; break;
case Screen::EditReps: if(b==Button::Up)session_.edit_reps(session_.current_reps()+1); else if(b==Button::Down)session_.edit_reps(std::max(0,session_.current_reps()-1)); else if(b==Button::Select||b==Button::Back)screen_=Screen::Workout; break;
case Screen::ConfirmSet: if(b==Button::Select){ auto dur=now_ms>=set_started_ms_?now_ms-set_started_ms_:0; session_.complete_set(now_ms,dur); if(session_.finished())screen_=Screen::AssessmentQuality; else screen_=Screen::Rest; } else if(b==Button::Back)screen_=Screen::Workout; break;
case Screen::Rest: if(b==Button::Select){session_.end_rest(now_ms);set_started_ms_=now_ms;screen_=Screen::Workout;} else if(b==Button::Up)screen_=Screen::EditWeight; else if(b==Button::Down)screen_=Screen::EditReps; break;
case Screen::AssessmentQuality: if(b==Button::Up)assessment_.workout_quality=std::min(5,assessment_.workout_quality+1); else if(b==Button::Down)assessment_.workout_quality=std::max(1,assessment_.workout_quality-1); else if(b==Button::Select)screen_=Screen::AssessmentStrength; break;
case Screen::AssessmentStrength: if(b==Button::Up)assessment_.strength=std::min(5,assessment_.strength+1); else if(b==Button::Down)assessment_.strength=std::max(1,assessment_.strength-1); else if(b==Button::Select)screen_=Screen::AssessmentEnergy; else if(b==Button::Back)screen_=Screen::AssessmentQuality; break;
case Screen::AssessmentEnergy: if(b==Button::Up)assessment_.energy=std::min(5,assessment_.energy+1); else if(b==Button::Down)assessment_.energy=std::max(1,assessment_.energy-1); else if(b==Button::Select)screen_=Screen::AssessmentRpe; else if(b==Button::Back)screen_=Screen::AssessmentStrength; break;
case Screen::AssessmentRpe: if(b==Button::Up)assessment_.session_rpe=std::min(10,assessment_.session_rpe+1); else if(b==Button::Down)assessment_.session_rpe=std::max(1,assessment_.session_rpe-1); else if(b==Button::Select)screen_=Screen::AssessmentPain; else if(b==Button::Back)screen_=Screen::AssessmentEnergy; break;
case Screen::AssessmentPain: if(b==Button::Up)assessment_.pain=std::min(10,assessment_.pain+1); else if(b==Button::Down)assessment_.pain=std::max(0,assessment_.pain-1); else if(b==Button::Select)screen_=Screen::AssessmentExpectation; else if(b==Button::Back)screen_=Screen::AssessmentRpe; break;
case Screen::AssessmentExpectation: if(b==Button::Up)expectation_index_=std::max(0,expectation_index_-1); else if(b==Button::Down)expectation_index_=std::min(2,expectation_index_+1); else if(b==Button::Select)finish_assessment(now_ms); else if(b==Button::Back)screen_=Screen::AssessmentPain; break;
case Screen::Summary: break; } }
void WorkoutController::finish_assessment(std::uint64_t now_ms){ static const char* k[]={"below_expected","as_expected","above_expected"}; assessment_.expectation=k[expectation_index_]; result_=session_.finish(now_ms,assessment_); result_ready_=true; screen_=Screen::Summary; }
ViewModel WorkoutController::view(std::uint64_t now_ms)const{ ViewModel v;v.screen=screen_; const auto&p=session_.plan(); const std::string ex=!session_.finished()?p.exercises.at(session_.exercise_index()).name:"Workout complete"; switch(screen_){
case Screen::Workout:v.title=ex;v.lines={"Set "+std::to_string(session_.set_index()+1)+"/"+std::to_string(p.exercises.at(session_.exercise_index()).sets.size()),"Weight: "+weight_string(session_.current_weight())+" lb","Reps: "+std::to_string(session_.current_reps())};v.footer="L1 wt  L2 reps  R1 done  R2 skip";break;
case Screen::EditWeight:v.title="Edit weight";v.lines={weight_string(session_.current_weight())+" lb"};v.footer="L1 +5  L2 -5  R1 save";break;
case Screen::EditReps:v.title="Edit reps";v.lines={std::to_string(session_.current_reps())};v.footer="L1 +1  L2 -1  R1 save";break;
case Screen::ConfirmSet:v.title="Complete set?";v.lines={ex,weight_string(session_.current_weight())+" lb x "+std::to_string(session_.current_reps())};v.footer="R1 confirm  R2 back";break;
case Screen::Rest:v.title="REST";v.lines={"Elapsed "+duration_string(session_.elapsed_rest_ms(now_ms)),"No countdown"};v.footer="R1 next set";break;
case Screen::AssessmentQuality:v.title="Workout quality";v.lines={std::to_string(assessment_.workout_quality)+" / 5"};v.footer="L1/L2 adjust  R1 next";break;
case Screen::AssessmentStrength:v.title="Strength";v.lines={std::to_string(assessment_.strength)+" / 5"};v.footer="L1/L2 adjust  R1 next";break;
case Screen::AssessmentEnergy:v.title="Energy";v.lines={std::to_string(assessment_.energy)+" / 5"};v.footer="L1/L2 adjust  R1 next";break;
case Screen::AssessmentRpe:v.title="Session RPE";v.lines={std::to_string(assessment_.session_rpe)+" / 10"};v.footer="L1/L2 adjust  R1 next";break;
case Screen::AssessmentPain:v.title="Pain/discomfort";v.lines={std::to_string(assessment_.pain)+" / 10"};v.footer="L1/L2 adjust  R1 next";break;
case Screen::AssessmentExpectation:{static const char* labels[]={"Below expected","As expected","Above expected"};v.title="Performance";v.lines={labels[expectation_index_]};v.footer="L1/L2 choose  R1 finish";break;}
case Screen::Summary:v.title="Saved";v.lines={"Reps: "+std::to_string(result_.total_reps),"Volume: "+weight_string(result_.training_volume_lb)+" lb"};v.footer="Workout complete";break;} return v; }
}
