﻿#pragma once

//
// ゲーム本編UI
//

#include "Task.hpp"
#include "CountExec.hpp"
#include "UICanvas.hpp"


namespace ngs {

class GameMain
  : public Task
{
  Event<Arguments>& event_;
  ConnectionHolder holder_;

  CountExec count_exec_;

  UI::Canvas canvas_;

  int mode_ = 0;


public:
  GameMain(const ci::JsonTree& params, Event<Arguments>& event, UI::Drawer& drawer) noexcept
    : event_(event),
      canvas_(event, drawer, params["ui.camera"], Params::load(params.getValueForKey<std::string>("gamemain.canvas")))
  {
    count_exec_.add(2.0, [this]() {
                           event_.signal("Game:Start", Arguments());
                           {
                             const auto& widget = canvas_.at("begin");
                             widget->enable(false);
                           }
                           {
                             const auto& widget = canvas_.at("main");
                             widget->enable();
                           }
                         });

    holder_ += event_.connect("pause:touch_ended",
                              [this](const Connection&, const Arguments& arg) noexcept
                              {
                                {
                                  const auto& widget = canvas_.at("main");
                                  widget->enable(false);
                                }
                                {
                                  const auto& widget = canvas_.at("pause_menu");
                                  widget->enable();
                                }
                                event_.signal("GameMain:pause", Arguments());
                              });
    
    holder_ += event_.connect("resume:touch_ended",
                              [this](const Connection&, const Arguments& arg) noexcept
                              {
                                {
                                  const auto& widget = canvas_.at("pause_menu");
                                  widget->enable(false);
                                }
                                {
                                  const auto& widget = canvas_.at("main");
                                  widget->enable();
                                }
                                event_.signal("GameMain:resume", Arguments());
                              });
    
    holder_ += event_.connect("abort:touch_ended",
                              [this](const Connection&, const Arguments& arg) noexcept
                              {
                                // ゲーム終了
                                event_.signal("GameMain:aborted", Arguments());
                                DOUT << "GameMain finished." << std::endl;
                              });

    // UI更新
    holder_ += event_.connect("Game:UI",
                              [this](const Connection&, const Arguments& arg) noexcept
                              {
                                char text[100];
                                u_int remainig_time = std::ceil(boost::any_cast<double>(arg.at("remaining_time")));
                                u_int minutes = remainig_time / 60;
                                u_int seconds = remainig_time % 60;
                                sprintf(text, "%d'%02d", minutes, seconds);

                                const auto& widget = canvas_.at("time_remain");
                                widget->setParam("text", std::string(text));

                                // 時間が10秒切ったら色を変える
                                ci::ColorA color(1, 1, 1, 1);
                                if (remainig_time <= 10) {
                                  color = ci::ColorA(1, 0, 0, 1);
                                }
                                widget->setParam("color", color);
                              });
  }

  ~GameMain() = default;


  bool update(const double current_time, const double delta_time) noexcept override
  {
    count_exec_.update(delta_time);

    return true;
  }

  void draw(const glm::ivec2& window_size) noexcept override
  {
    canvas_.draw();
  }

};

}