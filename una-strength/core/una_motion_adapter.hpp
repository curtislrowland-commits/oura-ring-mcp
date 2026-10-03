#pragma once
#include "una_strength/rep_counter.hpp"
#include "SDK/SensorLayer/DataParsers/SensorDataParserAccelerometer.hpp"
#include "SDK/SensorLayer/DataParsers/SensorDataParserGyroscope.hpp"
namespace una_strength {
struct UnaMotionAdapter {
    static bool accelerometer(const SDK::Sensor::DataView view, MotionSample& out){
        SDK::SensorDataParser::Accelerometer p(view);
        if(!p.isDataValid())return false;
        out.timestamp_ms=p.getTimestamp();out.ax=p.getX();out.ay=p.getY();out.az=p.getZ();return true;
    }
    static bool gyroscope(const SDK::Sensor::DataView view, MotionSample& out){
        SDK::SensorDataParser::Gyroscope p(view);
        if(!p.isDataValid())return false;
        out.timestamp_ms=p.getTimestamp();out.gx=p.getX();out.gy=p.getY();out.gz=p.getZ();return true;
    }
};
}
