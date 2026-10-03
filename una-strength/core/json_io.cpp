#include "una_strength/json_io.hpp"
#include "SDK/JSON/JsonStreamReader.hpp"
#include "SDK/JSON/JsonStreamWriter.hpp"
#include <cstdio>
#include <string_view>
#include <vector>

namespace una_strength {
namespace {
bool str(SDK::JsonStreamReader& r,const char*q,std::string& out){
    std::string_view v; if(!r.get(q,v)) return false; out.assign(v.data(),v.size()); return true;
}
}
bool JsonIO::loadWorkout(SDK::Interface::IFileSystem& fs,const char* path,WorkoutPlan& out,std::string& error){
    auto file=fs.file(path);
    if(!file||!file->exist()){error="workout file not found";return false;}
    if(!file->open(false,false)){error="cannot open workout file";return false;}
    const size_t size=file->size();
    if(size==0||size>65536){error="invalid workout file size";file->close();return false;}
    std::vector<char> data(size); size_t read=0;
    if(!file->read(data.data(),size,read)||read!=size){error="cannot read workout file";file->close();return false;}
    file->close();
    SDK::JsonStreamReader r(data.data(),data.size());
    if(!r.validate()){error="invalid JSON";return false;}
    uint32_t schema=1; r.get("schema_version",schema); out.schema_version=static_cast<int>(schema);
    if(!str(r,"workout_id",out.workout_id)||!str(r,"name",out.name)||!str(r,"scheduled_date",out.scheduled_date)){
        error="missing workout header fields";return false;
    }
    size_t exCount=0; if(!r.getArrayLength("exercises",exCount)||exCount==0||exCount>64){error="invalid exercises array";return false;}
    out.exercises.clear(); out.exercises.reserve(exCount);
    char q[128];
    for(size_t i=0;i<exCount;++i){
        ExercisePlan ex;
        std::snprintf(q,sizeof(q),"exercises[%zu].id",i); if(!str(r,q,ex.id)){error="exercise id missing";return false;}
        std::snprintf(q,sizeof(q),"exercises[%zu].name",i); if(!str(r,q,ex.name)){error="exercise name missing";return false;}
        std::snprintf(q,sizeof(q),"exercises[%zu].sets",i); size_t setCount=0;
        if(!r.getArrayLength(q,setCount)||setCount==0||setCount>30){error="invalid sets array";return false;}
        ex.sets.reserve(setCount);
        for(size_t j=0;j<setCount;++j){
            int32_t reps=0; double weight=0.0;
            std::snprintf(q,sizeof(q),"exercises[%zu].sets[%zu].target_reps",i,j); if(!r.get(q,reps)||reps<0||reps>1000){error="invalid target reps";return false;}
            std::snprintf(q,sizeof(q),"exercises[%zu].sets[%zu].target_weight_lb",i,j); if(!r.get(q,weight)||weight<0||weight>5000){error="invalid target weight";return false;}
            ex.sets.push_back({static_cast<int>(reps),weight});
        }
        out.exercises.push_back(std::move(ex));
    }
    return true;
}
bool JsonIO::saveResult(SDK::Interface::IFileSystem& fs,const char* path,const WorkoutResult& result,std::string& error){
    auto file=fs.file(path);
    if(!file||!file->open(true,true)){error="cannot open result file";return false;}
    SDK::JsonStreamWriter w(file.get());
    w.startMap();
    w.add("schema_version",static_cast<int32_t>(result.schema_version));
    w.add("workout_id",result.workout_id.c_str());
    w.add("workout_name",result.workout_name.c_str());
    w.add("date",result.date.c_str());
    w.add("started_at_unix_ms",result.started_at_unix_ms);
    w.add("ended_at_unix_ms",result.ended_at_unix_ms);
    w.add("duration_ms",result.duration_ms);
    w.add("total_reps",static_cast<int32_t>(result.total_reps));
    w.add("training_volume_lb",result.training_volume_lb);
    w.startArray("exercises");
    for(const auto& ex:result.exercises){
        w.startMap();
        w.add("id",ex.id.c_str());
        w.add("prescribed_name",ex.prescribed_name.c_str());
        w.add("actual_name",ex.actual_name.c_str());
        w.add("prescribed_order",static_cast<int32_t>(ex.prescribed_order));
        w.add("actual_order",static_cast<int32_t>(ex.actual_order));
        w.add("skipped",ex.skipped); w.add("substituted",ex.substituted); w.add("added",ex.added);
        w.startArray("sets");
        for(const auto& s:ex.sets){
            w.startMap();
            w.add("set_number",static_cast<int32_t>(s.set_number));
            w.add("prescribed_reps",static_cast<int32_t>(s.prescribed_reps));
            w.add("actual_reps",static_cast<int32_t>(s.actual_reps));
            w.add("prescribed_weight_lb",s.prescribed_weight_lb);
            w.add("actual_weight_lb",s.actual_weight_lb);
            w.add("set_duration_ms",s.set_duration_ms);
            w.add("rest_duration_ms",s.rest_duration_ms);
            w.add("skipped",s.skipped);
            w.add("auto_rep_count_used",s.auto_rep_count_used);
            w.add("auto_rep_count",static_cast<int32_t>(s.auto_rep_count));
            w.add("rep_count_corrected",s.rep_count_corrected);
            w.endMap();
        }
        w.endArray(); w.endMap();
    }
    w.endArray();
    w.startMap("heart_rate");
    w.add("available",result.heart_rate.available);
    w.add("sample_count",static_cast<int32_t>(result.heart_rate.sample_count));
    w.add("min_bpm",static_cast<int32_t>(result.heart_rate.min_bpm));
    w.add("max_bpm",static_cast<int32_t>(result.heart_rate.max_bpm));
    w.add("avg_bpm",result.heart_rate.avg_bpm);
    w.startArray("samples");
    for(const auto& h:result.heart_rate.samples){
        w.startMap();
        w.add("timestamp_ms",h.timestamp_ms);
        w.add("bpm",static_cast<int32_t>(h.bpm));
        w.add("trust_level",static_cast<double>(h.trust_level));
        w.endMap();
    }
    w.endArray();
    w.endMap();
    w.startMap("assessment");
    w.add("workout_quality",static_cast<int32_t>(result.assessment.workout_quality));
    w.add("strength",static_cast<int32_t>(result.assessment.strength));
    w.add("energy",static_cast<int32_t>(result.assessment.energy));
    w.add("session_rpe",static_cast<int32_t>(result.assessment.session_rpe));
    w.add("pain",static_cast<int32_t>(result.assessment.pain));
    w.add("expectation",result.assessment.expectation.c_str());
    w.startArray("tags"); for(const auto& tag:result.assessment.tags)w.add(tag.c_str()); w.endArray();
    w.add("notes",result.assessment.notes.c_str());
    w.endMap();
    w.endMap(); w.flush();
    const bool ok=!w.isError()&&file->flush(); file->close();
    if(!ok){error="result JSON write failed";return false;}
    return true;
}
}
