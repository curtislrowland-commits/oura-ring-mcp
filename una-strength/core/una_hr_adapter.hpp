#pragma once
#include "una_strength/heart_rate.hpp"
#include "SDK/SensorLayer/DataParsers/SensorDataParserHeartRate.hpp"
#include "SDK/SensorLayer/SensorDataBatch.hpp"

namespace una_strength {

// Thin adapter around UNA's documented HEART_RATE frame. The service owns the
// Sensor::Connection; this class only converts a DataBatch into our portable
// tracker format.
class UnaHeartRateAdapter {
public:
    explicit UnaHeartRateAdapter(HeartRateTracker& tracker) : tracker_(tracker) {}
    bool ingest(SDK::Sensor::DataBatch& data)
    {
        if (data.size() == 0) return false;
        SDK::SensorDataParser::HeartRate parser(data[0]);
        if (!parser.isDataValid()) return false;
        tracker_.add(parser.getTimestamp(), parser.getBpm(), parser.getTrustLevel());
        return true;
    }
private:
    HeartRateTracker& tracker_;
};

} // namespace una_strength
