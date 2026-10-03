#include "una_strength/controller.hpp"
#include "una_strength/heart_rate.hpp"
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
  WorkoutController c(makePlan(),1000);
  c.press(Button::Back,1100);
  assert(c.view(1100).screen==Screen::Actions);
  c.press(Button::Down,1110);c.press(Button::Down,1120);c.press(Button::Select,1130);
  assert(c.view(1130).lines.at(0)=="Set 1/3");
  c.press(Button::Back,1140);
  for(int i=0;i<4;++i)c.press(Button::Down,1150+i);
  c.press(Button::Select,1160);
  assert(c.view(1160).screen==Screen::ChooseSubstitute);
  c.press(Button::Down,1170);c.press(Button::Select,1180);
  assert(c.view(1180).title=="Bench Press");

  HeartRateTracker h;
  h.add(1000,0,3);h.add(2000,300,3);h.add(3000,120,0);
  assert(h.summary().sample_count==0);
  h.add(4000,120,3);h.add(5000,150,2);h.add(6000,180,1);
  auto s=h.summary();
  assert(s.available && s.sample_count==3 && s.min_bpm==120 && s.max_bpm==180);
  assert(std::fabs(s.avg_bpm-150.0)<0.001);
  std::cout<<"RC1_CONTROLLER_HR_ASSERTIONS_PASS\n";
}