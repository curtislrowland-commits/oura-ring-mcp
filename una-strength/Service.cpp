#include "Service.hpp"
#include "SDK/Messages/SensorLayerMessages.hpp"
#include "SDK/SensorLayer/DataParsers/SensorDataParserHeartRate.hpp"
#include "SDK/Messages/MessageGuard.hpp"
#define LOG_MODULE_PRX "UNA.Strength.Service"
#define LOG_MODULE_LEVEL LOG_LEVEL_INFO
#include "SDK/UnaLogger/Logger.h"

Service::Service(SDK::Kernel&)
    : mKernel(SDK::KernelProviderService::GetInstance().getKernel())
    , mGUIStarted(false)
    , mSensorHR(SDK::Sensor::Type::HEART_RATE, 1000, 2000)
{}

Service::~Service(){mSensorHR.disconnect();}

void Service::run()
{
    LOG_INFO("HR service started\n");
    const bool connected=mSensorHR.connect();
    LOG_INFO("UNA_HR_SENSOR_CONNECT %s\n",connected?"PASS":"UNAVAILABLE");

    while(true){
        SDK::MessageBase* msg=nullptr;
        if(!mKernel.comm.getMessage(msg,500)) continue;
        switch(msg->getType()){
            case SDK::MessageType::COMMAND_APP_STOP:
                mSensorHR.disconnect();
                mKernel.comm.releaseMessage(msg);
                return;
            case SDK::MessageType::COMMAND_APP_NOTIF_GUI_RUN:
                onStartGUI();
                break;
            case SDK::MessageType::COMMAND_APP_NOTIF_GUI_STOP:
                onStopGUI();
                break;
            case SDK::MessageType::EVENT_SENSOR_LAYER_DATA: {
                auto* event=static_cast<SDK::Message::Sensor::EventData*>(msg);
                SDK::Sensor::DataBatch data(event->data,event->count,event->stride);
                onSensorData(event->handle,data);
            } break;
            default: break;
        }
        mKernel.comm.releaseMessage(msg);
    }
}

void Service::onStartGUI(){mGUIStarted=true;}
void Service::onStopGUI(){mGUIStarted=false;}

void Service::onSensorData(std::uint16_t handle, SDK::Sensor::DataBatch& data)
{
    if(!mGUIStarted || !mSensorHR.matchesDriver(handle) || data.size()==0) return;
    SDK::SensorDataParser::HeartRate parser(data[0]);
    if(!parser.isDataValid()) return;
    SDK::send_msg<CustomMessage::HRValues>(
        mKernel, parser.getBpm(), parser.getTrustLevel(), parser.getTimestamp());
    LOG_INFO("UNA_HR_SENSOR_SAMPLE bpm=%.0f trust=%.1f ts=%u\n",
             parser.getBpm(),parser.getTrustLevel(),parser.getTimestamp());
}
