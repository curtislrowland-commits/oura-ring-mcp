#include "una_strength/session.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace una_strength;
static WorkoutPlan makePlan(){
  WorkoutPlan p;p.workout_id="rc1";p.name="RC1";p.scheduled_date="2026-10-03";
  p.exercises.push_back({"pendulum_squat","Pendulum Squat",{{5,55.0},{5,55.0}}});
  p.exercises.push_back({"bench_press","Bench Press",{{5,185.0}}});
  return p;
}
int main(){
  Session a(makePlan(),1000);
  a.skip_current_exercise(1100);
  assert(a.exercise_index()==1 && a.set_index()==0 && !a.finished());
  a.complete_set(2000,900);
  auto ar=a.finish(2100,SubjectiveAssessment{});
  assert(ar.exercises[0].skipped && ar.exercises[0].sets.size()==2);
  assert(ar.exercises[0].sets[0].skipped && ar.exercises[0].sets[1].skipped);
  assert(ar.total_reps==5 && std::fabs(ar.training_volume_lb-925.0)<0.001);

  Session b(makePlan(),1000);
  assert(b.add_set_current() && b.plan().exercises[0].sets.size()==3);
  assert(b.remove_current_set() && b.plan().exercises[0].sets.size()==2);
  b.substitute_current_exercise("shoulder_press","Shoulder Press");
  b.skip_current_exercise(1200);b.skip_current_exercise(1300);
  auto br=b.finish(1400,SubjectiveAssessment{});
  assert(br.exercises[0].substituted && br.exercises[0].actual_name=="Shoulder Press");

  Session c(makePlan(),1000);
  assert(c.move_current_exercise_later());
  assert(c.plan().exercises[0].name=="Bench Press");
  assert(c.add_exercise("seated_leg_curl","Seated Leg Curl",80.0,10));
  c.skip_current_exercise(1100);c.skip_current_exercise(1200);c.skip_current_exercise(1300);
  auto cr=c.finish(1400,SubjectiveAssessment{});
  assert(cr.exercises.size()==3 && cr.exercises[2].added);
  std::cout<<"RC1_SESSION_ASSERTIONS_PASS\n";
}