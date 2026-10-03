#pragma once
#include "SDK/Messages/MessageBase.hpp"
#include "SDK/Messages/MessageTypes.hpp"
#pragma pack(push,4)
namespace CustomMessage {
constexpr SDK::MessageType::Type HR_VALUES = 0x00000041;
struct HRValues : public SDK::MessageBase {
    float heartRate;
    float trustLevel;
    std::uint32_t timestampMs;
    HRValues():SDK::MessageBase(HR_VALUES),heartRate(0),trustLevel(0),timestampMs(0){}
    HRValues(float hr,float trust,std::uint32_t ts):HRValues(){heartRate=hr;trustLevel=trust;timestampMs=ts;}
};
}
#pragma pack(pop)
