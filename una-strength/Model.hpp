#ifndef MODEL_HPP
#define MODEL_HPP
#include "Commands.hpp"
#include "SDK/Interfaces/ICustomMessageHandler.hpp"
#include "SDK/Interfaces/IGuiLifeCycleCallback.hpp"
#include "SDK/Kernel/Kernel.hpp"
class ModelListener;
class Model : public SDK::Interface::IGuiLifeCycleCallback,
              public SDK::Interface::ICustomMessageHandler {
public:
    Model();
    void bind(ModelListener* listener){modelListener=listener;}
    void exitApp();
protected:
    ModelListener* modelListener;
    const SDK::Kernel& mKernel;
    void onStart() override;
    void onResume() override;
    void onSuspend() override;
    void onStop() override;
    bool customMessageHandler(SDK::MessageBase* msg) override;
};
#endif
