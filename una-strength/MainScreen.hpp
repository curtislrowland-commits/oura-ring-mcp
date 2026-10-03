#ifndef MAIN_SCREEN_HPP
#define MAIN_SCREEN_HPP
#include <cstdint>
#include <memory>
#include "lvgl.h"
#include "SDK/GUI/LVGL/Buttons.hpp"
#include "gui/model/Model.hpp"
#include "gui/model/ModelListener.hpp"
#include "una_strength/controller.hpp"
class MainScreen : public ModelListener {
public:
    explicit MainScreen(Model& model);
    ~MainScreen();
    MainScreen(const MainScreen&) = delete;
    MainScreen& operator=(const MainScreen&) = delete;
    lv_obj_t* root() const { return mRoot; }
private:
    void onKey(uint8_t code);
    static void keyEventCb(lv_event_t* e);
    void render();
    static std::uint64_t nowMs();
    Model& mModel;
    lv_obj_t* mRoot = nullptr;
    lv_obj_t* mTitle = nullptr;
    lv_obj_t* mBody = nullptr;
    lv_obj_t* mFooter = nullptr;
    std::unique_ptr<SDK::LVGL::Buttons> mButtons;
    una_strength::WorkoutPlan mPlan;
    std::unique_ptr<una_strength::WorkoutController> mController;
};
#endif
