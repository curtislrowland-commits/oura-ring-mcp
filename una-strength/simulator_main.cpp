#include <thread>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <SDL.h>
#include "SDK/Kernel/KernelProviderGUI.hpp"
#include "SDK/Kernel/KernelProviderService.hpp"
#include "SDK/Port/LVGL/LvglPort.hpp"
#include "SDK/Simulator/App/AppMessageCore.hpp"
#include "SDK/Simulator/App/KernelMessageDispatcher.hpp"
#include "SDK/Simulator/Kernel/Kernel.hpp"
#include "SDK/Simulator/Kernel/Mock/Backlight.hpp"
#include "SDK/Simulator/Kernel/Mock/Buzzer.hpp"
#include "SDK/Simulator/Kernel/Mock/System.hpp"
#include "SDK/Simulator/Kernel/Mock/Vibro.hpp"
#include "SDK/Simulator/LVGL/LvglHost.hpp"
#include "Service.hpp"

#define LOG_MODULE_PRX "main"
#define LOG_MODULE_LEVEL LOG_LEVEL_INFO
#include "SDK/UnaLogger/Logger.h"

extern "C" void una_lvgl_app_init(void);

static void tap(SDL_Keycode key)
{
    SDL_Event e{};
    e.type=SDL_KEYDOWN; e.key.keysym.sym=key; SDL_PushEvent(&e);
    std::this_thread::sleep_for(std::chrono::milliseconds(70));
    e.type=SDL_KEYUP; e.key.keysym.sym=key; SDL_PushEvent(&e);
    std::this_thread::sleep_for(std::chrono::milliseconds(350));
}

static void scheduledFlow()
{
    tap(SDLK_3); // launch Scheduled
    tap(SDLK_3); tap(SDLK_3); // set 1 confirm + complete
    tap(SDLK_3);               // leave manual rest
    tap(SDLK_3); tap(SDLK_3); // set 2 confirm + complete
    for(int i=0;i<6;++i) tap(SDLK_3); // assessment
}

static void freeFlow()
{
    tap(SDLK_2); // launch menu -> Free Workout
    tap(SDLK_3); // start Free
    tap(SDLK_3); // add Pendulum Squat

    // Set 1: edit 55 -> 60 lb, reps 5 -> 6.
    tap(SDLK_1); tap(SDLK_1); tap(SDLK_3);
    tap(SDLK_2); tap(SDLK_1); tap(SDLK_3);
    tap(SDLK_3); tap(SDLK_3); // confirm + complete
    tap(SDLK_3);              // rest -> options

    // Add set 2 and complete 60 x 6.
    tap(SDLK_3);
    tap(SDLK_3); tap(SDLK_3);
    tap(SDLK_3);              // rest -> options

    // Remove set 2, then add it back with reps changed 6 -> 5.
    tap(SDLK_2); tap(SDLK_3); // Remove last set
    tap(SDLK_1); tap(SDLK_3); // Add set
    tap(SDLK_2); tap(SDLK_2); tap(SDLK_3); // reps editor: -1, save
    tap(SDLK_3); tap(SDLK_3); // confirm + complete
    tap(SDLK_3);              // rest -> options

    // Add a second exercise: Bench Press.
    tap(SDLK_2); tap(SDLK_2); tap(SDLK_3); // Add exercise
    tap(SDLK_2); tap(SDLK_3);              // choose Bench Press
    tap(SDLK_3); tap(SDLK_3);              // 185 x 5
    tap(SDLK_3);                           // rest -> options

    // Finish workout.
    tap(SDLK_2); tap(SDLK_2); tap(SDLK_2); tap(SDLK_3);
    for(int i=0;i<6;++i) tap(SDLK_3);       // assessment
}

int main(int,char**)
{
    SDK::Simulator::Mock::SystemService serviceSystem;
    SDK::Simulator::Kernel serviceKernel("service");
    SDK::Simulator::Mock::SystemGUI guiSystem;
    SDK::Simulator::Kernel guiKernel("gui");

    SDK::App::MessageCore appMessageCore;
    SDK::App::DualAppComm& appComm=appMessageCore.getAppComm();
    serviceKernel.setIAppComm(appComm.getServiceComm());
    serviceKernel.setISystem(&serviceSystem);
    guiKernel.setIAppComm(appComm.getGuiComm());
    guiKernel.setISystem(&guiSystem);
    SDK::Simulator::KernelHolder::Create(guiKernel);

    Logger_init(serviceKernel.getKernel().log);
    SDK::KernelProviderService::CreateInstance(&serviceKernel.getKernel());
    SDK::KernelProviderGUI::CreateInstance(&guiKernel.getKernel());
    Service service(SDK::KernelProviderService::GetInstance().getKernel());

    SDK::Simulator::Mock::Backlight backlight;
    SDK::Simulator::Mock::Buzzer buzzer;
    SDK::Simulator::Mock::Vibro vibro;
    SDK::App::KernelMessageDispatcher dispatcher(appComm,appComm.getMsgManager(),vibro,backlight,buzzer);

    SDK::Simulator::LvglHost::Options options;
    options.title="UNA Strength";
    options.scale=1;
    SDK::Simulator::LvglHost host(appComm,serviceKernel.getKernel(),options);
    if(!host.init()) return 1;

    std::thread serviceThread(&Service::run,&service);
    std::thread dispatcherThread(&SDK::App::KernelMessageDispatcher::run,&dispatcher);
    SDK::LVGL::Port& port=SDK::LVGL::Port::GetInstance();
    port.init();
    una_lvgl_app_init();
    host.start();

    const char* mode=std::getenv("UNA_TEST_MODE");
    std::thread injector([mode]{
        std::this_thread::sleep_for(std::chrono::milliseconds(600));
        if(mode && std::strcmp(mode,"free")==0) freeFlow();
        else scheduledFlow();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        SDL_Event q{}; q.type=SDL_QUIT; SDL_PushEvent(&q);
    });

    host.run();
    injector.join();
    host.shutdown();
    serviceThread.join();
    dispatcherThread.join();
    LOG_INFO("Simulator finished\n");
    return 0;
}
