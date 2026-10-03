#include "una_strength/history.hpp"
#include "SDK/JSON/JsonStreamReader.hpp"
#include <algorithm>
#include <cstring>
#include <string_view>
#include <vector>

namespace una_strength {
namespace {
bool getString(SDK::JsonStreamReader& r, const char* query, std::string& out)
{
    std::string_view v;
    if (!r.get(query, v)) return false;
    out.assign(v.data(), v.size());
    return true;
}
}

bool WorkoutHistory::readEntry(SDK::Interface::IFileSystem& fs,
                               const char* path,
                               HistoryEntry& out)
{
    auto file=fs.file(path);
    if(!file || !file->exist() || !file->open(false,false)) return false;
    const size_t size=file->size();
    if(size==0 || size>131072){file->close();return false;}
    std::vector<char> data(size);
    size_t read=0;
    const bool ok=file->read(data.data(),size,read) && read==size;
    file->close();
    if(!ok) return false;

    SDK::JsonStreamReader r(data.data(),data.size());
    if(!r.validate()) return false;

    double started=0, ended=0, duration=0;
    int32_t totalReps=0, quality=0, rpe=0, pain=0;
    double volume=0;

    if(!getString(r,"workout_id",out.workout_id) ||
       !getString(r,"workout_name",out.workout_name) ||
       !getString(r,"date",out.date) ||
       !r.get("started_at_unix_ms",started) ||
       !r.get("ended_at_unix_ms",ended) ||
       !r.get("duration_ms",duration) ||
       !r.get("total_reps",totalReps) ||
       !r.get("training_volume_lb",volume)) return false;

    r.get("assessment.workout_quality",quality);
    r.get("assessment.session_rpe",rpe);
    r.get("assessment.pain",pain);
    getString(r,"assessment.expectation",out.expectation);

    out.path=path;
    out.started_at_unix_ms=static_cast<std::uint64_t>(started);
    out.ended_at_unix_ms=static_cast<std::uint64_t>(ended);
    out.duration_ms=static_cast<std::uint64_t>(duration);
    out.total_reps=static_cast<int>(totalReps);
    out.training_volume_lb=volume;
    out.workout_quality=static_cast<int>(quality);
    out.session_rpe=static_cast<int>(rpe);
    out.pain=static_cast<int>(pain);
    return true;
}

bool WorkoutHistory::load(SDK::Interface::IFileSystem& fs,
                          const char* directory,
                          std::vector<HistoryEntry>& out,
                          std::string& error)
{
    out.clear();
    auto dir=fs.dir(directory);
    if(!dir || !dir->exist() || !dir->open()){
        error="history directory unavailable";
        return false;
    }

    SDK::Interface::IFileSystem::ObjectInfo item{};
    while(dir->readNext(item)){
        if(item.isDir) continue;
        const char* dot=std::strrchr(item.name,'.');
        if(!dot || std::strcmp(dot,".json")!=0) continue;
        std::string path=std::string(directory);
        if(!path.empty() && path.back()!='/') path+='/';
        path+=item.name;
        HistoryEntry entry;
        if(readEntry(fs,path.c_str(),entry)) out.push_back(std::move(entry));
    }
    dir->close();

    std::sort(out.begin(),out.end(),[](const HistoryEntry& a,const HistoryEntry& b){
        if(a.ended_at_unix_ms!=b.ended_at_unix_ms)
            return a.ended_at_unix_ms>b.ended_at_unix_ms;
        if(a.started_at_unix_ms!=b.started_at_unix_ms)
            return a.started_at_unix_ms>b.started_at_unix_ms;
        return a.path>b.path;
    });

    if(out.empty()){
        error="no completed workouts";
        return false;
    }
    return true;
}

} // namespace una_strength
