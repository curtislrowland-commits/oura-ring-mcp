#include "gui/screens/MainScreen.hpp"
#include <chrono>
#include <string>
#include "SDK/GUI/Button.hpp"
#include "SDK/GUI/Color.hpp"
#include "SDK/GUI/LVGL/Draw.hpp"
#include "SDK/Kernel/KernelProviderGUI.hpp"
#include "gui/Assets.hpp"

#define LOG_MODULE_PRX "UNA.Strength"
#define LOG_MODULE_LEVEL LOG_LEVEL_INFO
#include "SDK/UnaLogger/Logger.h"

namespace Draw = SDK::LVGL::Draw;
using namespace una_strength;

std::uint64_t MainScreen::nowMs()
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

MainScreen::MainScreen(Model& model)
    : mModel(model)
{
    mRoot = lv_obj_create(nullptr);
    Draw::applyScreen(mRoot);
    lv_obj_add_event_cb(mRoot, &MainScreen::keyEventCb, LV_EVENT_KEY, this);

    mTitle = lv_label_create(mRoot);
    lv_obj_set_style_text_font(mTitle, &poppins_regular_18, 0);
    lv_obj_set_style_text_color(mTitle, lv_color_white(), 0);
    lv_obj_set_width(mTitle, 220);
    lv_obj_align(mTitle, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_set_style_text_align(mTitle, LV_TEXT_ALIGN_CENTER, 0);

    mBody = lv_label_create(mRoot);
    lv_obj_set_style_text_font(mBody, &poppins_regular_18, 0);
    lv_obj_set_style_text_color(mBody, lv_color_white(), 0);
    lv_obj_set_width(mBody, 220);
    lv_obj_align(mBody, LV_ALIGN_CENTER, 0, -5);
    lv_obj_set_style_text_align(mBody, LV_TEXT_ALIGN_CENTER, 0);

    mFooter = lv_label_create(mRoot);
    lv_obj_set_style_text_font(mFooter, &poppins_regular_18, 0);
    lv_obj_set_style_text_color(mFooter, lv_color_white(), 0);
    lv_obj_set_width(mFooter, 220);
    lv_obj_align(mFooter, LV_ALIGN_BOTTOM_MID, 0, -22);
    lv_obj_set_style_text_align(mFooter, LV_TEXT_ALIGN_CENTER, 0);

    mButtons = std::make_unique<SDK::LVGL::Buttons>(mRoot);
    mButtons->set(SDK::LVGL::Buttons::WHITE, SDK::LVGL::Buttons::WHITE,
                  SDK::LVGL::Buttons::WHITE, SDK::LVGL::Buttons::WHITE);

    bind(&mModel);
    mModel.bind(this);
    renderLaunchMenu();
    LOG_INFO("UNA_STRENGTH_SCREEN_READY\n");
}

MainScreen::~MainScreen()
{
    mModel.bind(nullptr);
    lv_obj_delete(mRoot);
}

void MainScreen::startScheduled()
{
    auto& fs = SDK::KernelProviderGUI::GetInstance().getKernel().fs;
    std::string error;
    const std::string today = WorkoutRepository::localDate();
    std::string selectedPath;
    mWorkoutLoaded = WorkoutRepository::loadScheduledForDate(
        fs, "workouts", today, mPlan, selectedPath, error);

    if (!mWorkoutLoaded) {
        LOG_ERROR("UNA_STRENGTH_JSON_IMPORT_FAIL error=%s\n", error.c_str());
        mPlan.workout_id = "IMPORT_FAILED";
        mPlan.name = "Workout load failed";
        mPlan.scheduled_date = "1970-01-01";
        ExercisePlan ex;
        ex.id = "error";
        ex.name = "Import error";
        ex.sets.push_back({1, 0.0});
        mPlan.exercises.push_back(ex);
    } else {
        LOG_INFO("UNA_STRENGTH_DATE_SELECT_PASS date=%s path=%s id=%s\n",
                 today.c_str(), selectedPath.c_str(), mPlan.workout_id.c_str());
        LOG_INFO("UNA_STRENGTH_JSON_IMPORT_PASS id=%s name=%s exercises=%u\n",
                 mPlan.workout_id.c_str(), mPlan.name.c_str(),
                 static_cast<unsigned>(mPlan.exercises.size()));
    }

    mController = std::make_unique<WorkoutController>(mPlan, nowMs());
    mMode = LaunchMode::Scheduled;
    mResultSaved = false;
    render();
}

void MainScreen::startFree()
{
    const std::string today = WorkoutRepository::localDate();
    mFreeController = std::make_unique<FreeWorkoutController>(today, nowMs());
    mMode = LaunchMode::Free;
    mResultSaved = false;
    LOG_INFO("UNA_FREE_WORKOUT_START date=%s\n", today.c_str());
    render();
}

void MainScreen::startHistory()
{
    auto& fs = SDK::KernelProviderGUI::GetInstance().getKernel().fs;
    std::string error;
    if (!WorkoutHistory::load(fs, "results", mHistory, error)) {
        mHistory.clear();
        mMode = LaunchMode::HistoryList;
        lv_label_set_text(mTitle, "Workout History");
        lv_label_set_text(mBody, "No completed workouts");
        lv_label_set_text(mFooter, "R2 back");
        LOG_INFO("UNA_HISTORY_EMPTY error=%s\n", error.c_str());
        return;
    }
    mHistoryIndex = 0;
    mMode = LaunchMode::HistoryList;
    LOG_INFO("UNA_HISTORY_LOAD_PASS count=%u newest=%s\n",
             static_cast<unsigned>(mHistory.size()),
             mHistory.front().workout_name.c_str());
    renderHistory();
}

void MainScreen::renderHistory()
{
    if (mHistory.empty()) {
        lv_label_set_text(mTitle, "Workout History");
        lv_label_set_text(mBody, "No completed workouts");
        lv_label_set_text(mFooter, "R2 back");
        return;
    }

    const auto& e = mHistory.at(mHistoryIndex);
    if (mMode == LaunchMode::HistoryList) {
        std::string body = e.date + "\n" + e.workout_name;
        lv_label_set_text(mTitle, "Workout History");
        lv_label_set_text(mBody, body.c_str());
        lv_label_set_text(mFooter, "L1/L2 browse R1 open R2 back");
        LOG_INFO("UNA_HISTORY_LIST index=%u name=%s date=%s\n",
                 static_cast<unsigned>(mHistoryIndex), e.workout_name.c_str(), e.date.c_str());
    } else {
        std::string body = e.date + "\nReps: " + std::to_string(e.total_reps) +
                           "\nVolume: " + std::to_string(static_cast<int>(e.training_volume_lb)) +
                           " lb\nRPE: " + std::to_string(e.session_rpe);
        lv_label_set_text(mTitle, e.workout_name.c_str());
        lv_label_set_text(mBody, body.c_str());
        lv_label_set_text(mFooter, "R2 back");
        LOG_INFO("UNA_HISTORY_DETAIL_PASS name=%s reps=%d volume=%.0f rpe=%d\n",
                 e.workout_name.c_str(), e.total_reps, e.training_volume_lb, e.session_rpe);
    }
}

void MainScreen::renderLaunchMenu()
{
    static const char* options[] = {"Scheduled Workout", "Free Workout", "Workout History"};
    lv_label_set_text(mTitle, "UNA Strength");
    lv_label_set_text(mBody, options[mLaunchIndex]);
    lv_label_set_text(mFooter, "L1/L2 choose  R1 start");
    LOG_INFO("UNA_LAUNCH_MENU option=%s\n", options[mLaunchIndex]);
}

void MainScreen::keyEventCb(lv_event_t* e)
{
    auto* self = static_cast<MainScreen*>(lv_event_get_user_data(e));
    self->onKey(static_cast<uint8_t>(lv_event_get_key(e)));
}

void MainScreen::onKey(uint8_t code)
{
    namespace Btn = SDK::GUI::Button;
    Button b;
    bool click = true;
    switch (code) {
        case Btn::L1: b = Button::Up; break;
        case Btn::L2: b = Button::Down; break;
        case Btn::R1: b = Button::Select; break;
        case Btn::R2: b = Button::Back; break;
        default: click = false; break;
    }
    if (!click) return;

    if (mMode == LaunchMode::Menu) {
        if (b == Button::Up) {
            mLaunchIndex = (mLaunchIndex + 2) % 3;
            renderLaunchMenu();
        } else if (b == Button::Down) {
            mLaunchIndex = (mLaunchIndex + 1) % 3;
            renderLaunchMenu();
        } else if (b == Button::Select) {
            if (mLaunchIndex == 0) startScheduled();
            else if (mLaunchIndex == 1) startFree();
            else startHistory();
        }
        return;
    }

    if (mMode == LaunchMode::HistoryList) {
        if (b == Button::Back) {
            mMode = LaunchMode::Menu;
            renderLaunchMenu();
        } else if (!mHistory.empty() && b == Button::Up) {
            mHistoryIndex = (mHistoryIndex + mHistory.size() - 1) % mHistory.size();
            renderHistory();
        } else if (!mHistory.empty() && b == Button::Down) {
            mHistoryIndex = (mHistoryIndex + 1) % mHistory.size();
            renderHistory();
        } else if (!mHistory.empty() && b == Button::Select) {
            mMode = LaunchMode::HistoryDetail;
            renderHistory();
        }
        return;
    }

    if (mMode == LaunchMode::HistoryDetail) {
        if (b == Button::Back) {
            mMode = LaunchMode::HistoryList;
            renderHistory();
        }
        return;
    }

    if (mMode == LaunchMode::Scheduled) {
        mController->press(b, nowMs());
    } else if (mMode == LaunchMode::Free) {
        mFreeController->press(b, nowMs());
    }
    render();
}

void MainScreen::render()
{
    ViewModel v;
    if (mMode == LaunchMode::Scheduled) v = mController->view(nowMs());
    else if (mMode == LaunchMode::Free) v = mFreeController->view(nowMs());
    else if (mMode == LaunchMode::HistoryList || mMode == LaunchMode::HistoryDetail) { renderHistory(); return; }
    else { renderLaunchMenu(); return; }

    std::string body;
    for (std::size_t i = 0; i < v.lines.size(); ++i) {
        if (i) body += "\n";
        body += v.lines[i];
    }

    lv_label_set_text(mTitle, v.title.c_str());
    lv_label_set_text(mBody, body.c_str());
    lv_label_set_text(mFooter, v.footer.c_str());
    LOG_INFO("UNA_VIEW title=%s body=%s\n", v.title.c_str(), body.c_str());

    if (mMode == LaunchMode::Scheduled && v.screen == Screen::Rest) {
        LOG_INFO("UNA_STRENGTH_REST_REACHED\n");
    }
    if (mMode == LaunchMode::Free && mFreeController->screen() == FreeScreen::Rest) {
        LOG_INFO("UNA_FREE_REST_REACHED\n");
    }

    if (mResultSaved) return;

    const WorkoutResult* result = nullptr;

    if (mMode == LaunchMode::Scheduled && v.screen == Screen::Summary) {
        result = mController->result();
    } else if (mMode == LaunchMode::Free && mFreeController->screen() == FreeScreen::Summary) {
        result = mFreeController->result();
    } else {
        return;
    }

    if (!result) {
        LOG_ERROR("UNA_STRENGTH_JSON_EXPORT_FAIL error=no result\n");
        return;
    }

    auto& fs = SDK::KernelProviderGUI::GetInstance().getKernel().fs;
    fs.mkdir("results");
    const std::string resultPath = "results/" + result->date + "-" +
        std::to_string(result->started_at_unix_ms) + "-" +
        (mMode == LaunchMode::Free ? "free.json" : "scheduled.json");
    std::string error;
    if (!JsonIO::saveResult(fs, resultPath.c_str(), *result, error)) {
        LOG_ERROR("UNA_STRENGTH_JSON_EXPORT_FAIL error=%s\n", error.c_str());
        return;
    }

    mResultSaved = true;
    LOG_INFO("UNA_STRENGTH_JSON_EXPORT_PASS path=%s reps=%d volume=%.0f\n",
             resultPath.c_str(), result->total_reps, result->training_volume_lb);

    if (mMode == LaunchMode::Free) {
        if (result->workout_name == "Free Workout" &&
            result->exercises.size() == 2 &&
            result->total_reps == 16 &&
            result->training_volume_lb == 1585.0) {
            LOG_INFO("UNA_FREE_WORKOUT_PASS exercises=%u reps=%d volume=%.0f\n",
                     static_cast<unsigned>(result->exercises.size()),
                     result->total_reps, result->training_volume_lb);
        } else {
            LOG_ERROR("UNA_FREE_WORKOUT_FAIL exercises=%u reps=%d volume=%.0f\n",
                      static_cast<unsigned>(result->exercises.size()),
                      result->total_reps, result->training_volume_lb);
        }
    } else if (mWorkoutLoaded &&
               result->workout_id == "scheduled-today-pass" &&
               result->total_reps == 10 &&
               result->training_volume_lb == 550.0) {
        LOG_INFO("UNA_STRENGTH_ROUNDTRIP_PASS\n");
    } else {
        LOG_ERROR("UNA_STRENGTH_ROUNDTRIP_FAIL\n");
    }
}
