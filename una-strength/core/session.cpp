#include "una_strength/session.hpp"
#include <algorithm>

namespace una_strength {
Session::Session(const WorkoutPlan&p,std::uint64_t start_ms):plan_(p){ result_.workout_id=p.workout_id;result_.workout_name=p.name;result_.date=p.scheduled_date;result_.started_at_unix_ms=start_ms; for(size_t i=0;i<p.exercises.size();++i){CompletedExercise e;e.id=p.exercises[i].id;e.prescribed_name=p.exercises[i].name;e.actual_name=p.exercises[i].name;e.prescribed_order=static_cast<int>(i+1);e.actual_order=static_cast<int>(i+1);result_.exercises.push_back(e);} if(p.exercises.empty())finished_=true;else load_current(); }
void Session::load_current(){ if(finished_)return; const auto&sp=plan_.exercises.at(ex_).sets.at(set_);edit_reps_=sp.target_reps;edit_weight_=sp.target_weight_lb; }
void Session::edit_reps(int r){edit_reps_=std::max(0,std::min(1000,r));}
void Session::edit_weight(double w){edit_weight_=std::max(0.0,std::min(5000.0,w));}
std::uint64_t Session::elapsed_rest_ms(std::uint64_t now)const{return last_set_end_ms_?now-last_set_end_ms_:0;}
void Session::end_rest(std::uint64_t now){
    if(!last_set_end_ms_) return;
    for(auto ex_it=result_.exercises.rbegin(); ex_it!=result_.exercises.rend(); ++ex_it){
        if(!ex_it->sets.empty()){
            ex_it->sets.back().rest_duration_ms=elapsed_rest_ms(now);
            return;
        }
    }
}
void Session::complete_set(std::uint64_t now,std::uint64_t dur,int auto_count){if(finished_)return;const auto&sp=plan_.exercises[ex_].sets[set_];CompletedSet s;s.set_number=static_cast<int>(set_+1);s.prescribed_reps=sp.target_reps;s.prescribed_weight_lb=sp.target_weight_lb;s.actual_reps=edit_reps_;s.actual_weight_lb=edit_weight_;s.set_duration_ms=dur;s.rest_duration_ms=0;if(auto_count>=0){s.auto_rep_count_used=true;s.auto_rep_count=auto_count;s.rep_count_corrected=(auto_count!=edit_reps_);}result_.exercises[ex_].sets.push_back(s);result_.total_reps+=s.actual_reps;result_.training_volume_lb+=s.actual_reps*s.actual_weight_lb;last_set_end_ms_=now;advance();}
void Session::skip_current_set(std::uint64_t now){if(finished_)return;const auto&sp=plan_.exercises[ex_].sets[set_];CompletedSet s;s.set_number=static_cast<int>(set_+1);s.prescribed_reps=sp.target_reps;s.prescribed_weight_lb=sp.target_weight_lb;s.skipped=true;s.rest_duration_ms=0;result_.exercises[ex_].sets.push_back(s);last_set_end_ms_=now;advance();}
void Session::substitute_current_exercise(const std::string&id,const std::string&name){auto&e=result_.exercises.at(ex_);e.id=id;e.actual_name=name;e.substituted=true;}
void Session::skip_current_exercise(std::uint64_t now){
    if(finished_) return;
    const std::size_t target=ex_;
    result_.exercises.at(target).skipped=true;
    while(!finished_ && ex_==target) skip_current_set(now);
}
bool Session::add_set_current(){
    if(finished_) return false;
    auto& sets=plan_.exercises.at(ex_).sets;
    if(sets.empty()) return false;
    sets.push_back(sets.back());
    return true;
}
bool Session::remove_current_set(){
    if(finished_) return false;
    auto& sets=plan_.exercises.at(ex_).sets;
    if(sets.size()<=1) return false;
    if(set_>=sets.size()) return false;
    sets.erase(sets.begin()+static_cast<std::ptrdiff_t>(set_));
    if(set_>=sets.size()) set_=sets.size()-1;
    load_current();
    return true;
}
bool Session::add_exercise(const std::string&id,const std::string&name,double weight,int reps){
    ExercisePlan p;p.id=id;p.name=name;p.sets.push_back({reps,weight});plan_.exercises.push_back(p);
    CompletedExercise e;e.id=id;e.prescribed_name="";e.actual_name=name;e.prescribed_order=0;e.actual_order=static_cast<int>(result_.exercises.size()+1);e.added=true;
    result_.exercises.push_back(e);return true;
}
bool Session::move_current_exercise_later(){
    if(finished_ || ex_+1>=plan_.exercises.size()) return false;
    if(!result_.exercises.at(ex_).sets.empty()) return false;
    std::swap(plan_.exercises[ex_],plan_.exercises[ex_+1]);
    std::swap(result_.exercises[ex_],result_.exercises[ex_+1]);
    result_.exercises[ex_].actual_order=static_cast<int>(ex_+1);
    result_.exercises[ex_+1].actual_order=static_cast<int>(ex_+2);
    load_current();return true;
}
void Session::advance(){++set_;if(set_>=plan_.exercises[ex_].sets.size()){set_=0;++ex_;if(ex_>=plan_.exercises.size()){finished_=true;return;}}load_current();}
WorkoutResult Session::finish(std::uint64_t end,const SubjectiveAssessment&a){if(!finished_)return result_;result_.ended_at_unix_ms=end;result_.duration_ms=end-result_.started_at_unix_ms;result_.assessment=a;return result_;}
}
