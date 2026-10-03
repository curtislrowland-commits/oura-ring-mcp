#include "una_strength/rep_counter.hpp"
#include "una_strength/session.hpp"
#include <cassert>
#include <iostream>
using namespace una_strength;

static void feedRep(RepCounter& c, std::uint64_t t, MotionSignal signal){
  MotionSample s{};
  s.timestamp_ms=t;
  if(signal==MotionSignal::AccelY){s.ay=12.8f;c.feed(s);s.timestamp_ms=t+200;s.ay=11.5f;c.feed(s);s.timestamp_ms=t+500;s.ay=9.7f;c.feed(s);}
  else if(signal==MotionSignal::AccelZ){s.az=12.8f;c.feed(s);s.timestamp_ms=t+180;s.az=11.2f;c.feed(s);s.timestamp_ms=t+450;s.az=9.7f;c.feed(s);}
  else {s.gy=60.0f;c.feed(s);s.timestamp_ms=t+150;s.gy=30.0f;c.feed(s);s.timestamp_ms=t+350;s.gy=10.0f;c.feed(s);}
}

int main(){
  for(const auto& id : {"pendulum_squat","shoulder_press","bench_press"}){
    RepProfile p;assert(RepCounter::profileFor(id,p));
    RepCounter c(p);
    for(int i=0;i<5;++i) feedRep(c,1000+i*1500,p.signal);
    assert(c.count()==5);

    MotionSample noise{};
    for(int i=0;i<20;++i){
      noise.timestamp_ms=10000+i*100;
      noise.ay=10.5f;noise.az=10.5f;noise.gy=20.0f;
      c.feed(noise);
    }
    assert(c.count()==5);
  }

  RepProfile unknown;
  assert(!RepCounter::profileFor("unknown_exercise",unknown));

  WorkoutPlan p;
  p.workout_id="auto";p.name="Auto";p.scheduled_date="2026-10-03";
  p.exercises.push_back({"bench_press","Bench Press",{{5,185.0}}});
  Session s(p,1000);
  s.edit_reps(4);
  s.complete_set(2000,1000,5);
  auto result=s.finish(2100,SubjectiveAssessment{});
  const auto& set=result.exercises[0].sets[0];
  assert(set.actual_reps==4);
  assert(set.auto_rep_count_used);
  assert(set.auto_rep_count==5);
  assert(set.rep_count_corrected);
  std::cout<<"RC1_REP_COUNTER_ASSERTIONS_PASS\n";
}