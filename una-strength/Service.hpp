#ifndef __SERVICE_HPP__
#define __SERVICE_HPP__
#include "Commands.hpp"
#include "SDK/Kernel/KernelProviderService.hpp"
#include "SDK/SensorLayer/SensorConnection.hpp"
#include "SDK/SensorLayer/SensorDataBatch.hpp"

class Service {
public:
    explicit Service(SDK::Kernel& kernel);
    ~Service();
    void run();
private:
    SDK::Kernel& mKernel;
    bool mGUIStarted;
    SDK::Sensor::Connection mSensorHR;
    void onStartGUI();
    void onStopGUI();
    void onSensorData(std::uint16_t handle, SDK::Sensor::DataBatch& data);
};
#endif
