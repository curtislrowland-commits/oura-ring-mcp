#ifndef MAIN_SCREEN_HPP
#define MAIN_SCREEN_HPP
#include <cstdint>
#include <memory>
#include "lvgl.h"
#include "SDK/GUI/LVGL/Buttons.hpp"
#include "gui/model/Model.hpp"
#include "gui/model/ModelListener.hpp"
#include "una_strength/controller.hpp"
#include "una_strength/json_io.hpp"
#include "una_strength/workout_repository.hpp"
#include "una_strength/free_controller.hpp"
#include "una_strength/history.hpp"
#include "una_strength/heart_rate.hpp"
#include <vector>
class MainScreen : public ModelListener {
public:
    explicit MainScreen(Model& model);
    ~MainScreen();
    MainScreen(const MainScreen&) = delete;
    MainScreen& operator=(const MainScreen&) = delete;
    lv_obj_t* root() const { return mRoot; }
    void updateHR(float bpm, float trustLevel, std::uint32_t timestampMs) override;
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
    enum class LaunchMode { Menu, Scheduled, ScheduledError, Free, HistoryList, HistoryDetail };
    LaunchMode mMode = LaunchMode::Menu;
    int mLaunchIndex = 0;
    std::vector<una_strength::HistoryEntry> mHistory;
    std::size_t mHistoryIndex = 0;
    una_strength::HeartRateTracker mHeartRate;
    una_strength::WorkoutPlan mPlan;
    std::unique_ptr<una_strength::WorkoutController> mController;
    std::unique_ptr<una_strength::FreeWorkoutController> mFreeController;
    bool mResultSaved = false;
    bool mWorkoutLoaded = false;
    void startScheduled();
    void startFree();
    void startHistory();
    void renderHistory();
    void renderLaunchMenu();
};
#endif
