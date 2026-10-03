#include "gui/screens/MainScreen.hpp"
#include <chrono>
#include <string>
#include "SDK/GUI/Button.hpp"
#include "SDK/GUI/Color.hpp"
#include "SDK/GUI/LVGL/Draw.hpp"
#include "gui/Assets.hpp"
#define LOG_MODULE_PRX "UNA.Strength"
#define LOG_MODULE_LEVEL LOG_LEVEL_INFO
#include "SDK/UnaLogger/Logger.h"
namespace Draw = SDK::LVGL::Draw;
using namespace una_strength;
std::uint64_t MainScreen::nowMs(){using namespace std::chrono;return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();}
MainScreen::MainScreen(Model& model):mModel(model){
    mPlan.workout_id="una-sim-smoke";mPlan.name="UNA Strength Simulator Smoke";mPlan.scheduled_date="2026-10-03";
    ExercisePlan ex;ex.id="pendulum_squat";ex.name="Pendulum Squat";ex.sets.push_back({5,55.0});ex.sets.push_back({5,55.0});mPlan.exercises.push_back(ex);
    mController=std::make_unique<WorkoutController>(mPlan,nowMs());
    mRoot=lv_obj_create(nullptr);Draw::applyScreen(mRoot);lv_obj_add_event_cb(mRoot,&MainScreen::keyEventCb,LV_EVENT_KEY,this);
    mTitle=lv_label_create(mRoot);lv_obj_set_style_text_font(mTitle,&poppins_regular_18,0);lv_obj_set_style_text_color(mTitle,lv_color_white(),0);lv_obj_set_width(mTitle,220);lv_obj_align(mTitle,LV_ALIGN_TOP_MID,0,20);lv_obj_set_style_text_align(mTitle,LV_TEXT_ALIGN_CENTER,0);
    mBody=lv_label_create(mRoot);lv_obj_set_style_text_font(mBody,&poppins_regular_18,0);lv_obj_set_style_text_color(mBody,lv_color_white(),0);lv_obj_set_width(mBody,220);lv_obj_align(mBody,LV_ALIGN_CENTER,0,-5);lv_obj_set_style_text_align(mBody,LV_TEXT_ALIGN_CENTER,0);
    mFooter=lv_label_create(mRoot);lv_obj_set_style_text_font(mFooter,&poppins_regular_18,0);lv_obj_set_style_text_color(mFooter,lv_color_white(),0);lv_obj_set_width(mFooter,220);lv_obj_align(mFooter,LV_ALIGN_BOTTOM_MID,0,-22);lv_obj_set_style_text_align(mFooter,LV_TEXT_ALIGN_CENTER,0);
    mButtons=std::make_unique<SDK::LVGL::Buttons>(mRoot);mButtons->set(SDK::LVGL::Buttons::WHITE,SDK::LVGL::Buttons::WHITE,SDK::LVGL::Buttons::WHITE,SDK::LVGL::Buttons::WHITE);
    bind(&mModel);mModel.bind(this);render();LOG_INFO("UNA_STRENGTH_SCREEN_READY\n");
}
MainScreen::~MainScreen(){mModel.bind(nullptr);lv_obj_delete(mRoot);}
void MainScreen::keyEventCb(lv_event_t* e){auto*self=static_cast<MainScreen*>(lv_event_get_user_data(e));self->onKey(static_cast<uint8_t>(lv_event_get_key(e)));}
void MainScreen::onKey(uint8_t code){namespace Btn=SDK::GUI::Button;Button b;bool click=true;switch(code){case Btn::L1:b=Button::Up;break;case Btn::L2:b=Button::Down;break;case Btn::R1:b=Button::Select;break;case Btn::R2:b=Button::Back;break;default:click=false;break;}if(!click)return;mController->press(b,nowMs());render();}
void MainScreen::render(){auto v=mController->view(nowMs());std::string body;for(std::size_t i=0;i<v.lines.size();++i){if(i)body+="\n";body+=v.lines[i];}lv_label_set_text(mTitle,v.title.c_str());lv_label_set_text(mBody,body.c_str());lv_label_set_text(mFooter,v.footer.c_str());LOG_INFO("UNA_VIEW title=%s body=%s\n",v.title.c_str(),body.c_str());if(v.screen==Screen::Rest)LOG_INFO("UNA_STRENGTH_REST_REACHED\n");if(v.screen==Screen::Summary){const auto*r=mController->result();if(r&&r->total_reps==10&&r->training_volume_lb==550.0)LOG_INFO("UNA_STRENGTH_SMOKE_PASS reps=%d volume=%.0f\n",r->total_reps,r->training_volume_lb);else LOG_ERROR("UNA_STRENGTH_SMOKE_FAIL\n");}}
