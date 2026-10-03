#include "gui/model/Model.hpp"
#include "gui/model/ModelListener.hpp"
#include "SDK/Kernel/KernelProviderGUI.hpp"
#include "SDK/Port/LVGL/LvglPort.hpp"
#define LOG_MODULE_PRX "UNA.Strength.Model"
#define LOG_MODULE_LEVEL LOG_LEVEL_INFO
#include "SDK/UnaLogger/Logger.h"

Model::Model():modelListener(nullptr),mKernel(SDK::KernelProviderGUI::GetInstance().getKernel())
{
    SDK::LVGL::Port::GetInstance().setAppLifeCycleCallback(this);
    SDK::LVGL::Port::GetInstance().setCustomMessageHandler(this);
}
void Model::exitApp(){SDK::LVGL::Port::GetInstance().setAppLifeCycleCallback(nullptr);SDK::LVGL::Port::GetInstance().setCustomMessageHandler(nullptr);mKernel.sys.exit();}
void Model::onStart(){}
void Model::onResume(){}
void Model::onSuspend(){}
void Model::onStop(){}
bool Model::customMessageHandler(SDK::MessageBase* msg)
{
    if(!modelListener) return true;
    if(msg->getType()==CustomMessage::HR_VALUES){
        auto* m=static_cast<CustomMessage::HRValues*>(msg);
        modelListener->updateHR(m->heartRate,m->trustLevel,m->timestampMs);
    }
    return true;
}
