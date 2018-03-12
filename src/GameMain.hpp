﻿#pragma once

//
// ゲーム本編UI
//

#include "Task.hpp"
#include "CountExec.hpp"
#include "UICanvas.hpp"
#include "TweenUtil.hpp"


namespace ngs {

class GameMain
  : public Task
{
  Event<Arguments>& event_;
  ConnectionHolder holder_;

  CountExec count_exec_;

  UI::Canvas canvas_;

  bool active_ = true;


public:
  GameMain(const ci::JsonTree& params, Event<Arguments>& event, UI::Drawer& drawer, TweenCommon& tween_common) noexcept
    : event_(event),
      canvas_(event, drawer, tween_common,
              params["ui.camera"],
              Params::load(params.getValueForKey<std::string>("gamemain.canvas")),
              Params::load(params.getValueForKey<std::string>("gamemain.tweens")))
  {
    DOUT << "GameMain::GameMain" << std::endl;
    startTimelineSound(event, params, "gamemain.se");
    
    // ゲーム開始演出 
    count_exec_.add(2.6,
                    [this]() noexcept
                    {
                      event_.signal("Game:Start", Arguments());
                    });

    holder_ += event_.connect("pause:touch_ended",
                              [this](const Connection&, const Arguments&) noexcept
                              {
                                // Pause開始
                                canvas_.active(false);
                                event_.signal("GameMain:pause", Arguments());
                                canvas_.startTween("pause");

                                count_exec_.add(1.2,
                                                [this]() noexcept
                                                {
                                                  canvas_.active();
                                                });
                              });
    
    holder_ += event_.connect("resume:touch_ended",
                              [this](const Connection&, const Arguments&) noexcept
                              {
                                // Game続行(時間差で演出)
                                canvas_.active(false);
                                canvas_.startTween("resume");
                                count_exec_.add(1.2,
                                                [this]() noexcept
                                                {
                                                  canvas_.active();
                                                  event_.signal("GameMain:resume", Arguments());
                                                });
                              });
    
    holder_ += event_.connect("abort:touch_ended",
                              [this](const Connection&, const Arguments&) noexcept
                              {
                                // ゲーム終了
                                canvas_.active(false);
                                canvas_.startTween("abort");
                                count_exec_.add(0.6,
                                                [this]() noexcept
                                                {
                                                  event_.signal("Game:Aborted", Arguments());
                                                });
                                count_exec_.add(1.2,
                                                [this]() noexcept
                                                {
                                                  active_ = false;
                                                });
                                DOUT << "GameMain finished." << std::endl;
                              });

    // UI更新
    holder_ += event_.connect("Game:UI",
                              [this](const Connection&, const Arguments& arg) noexcept
                              {
                                char text[100];
                                auto remaining_time = boost::any_cast<double>(arg.at("remaining_time"));
                                if (remaining_time < 10.0)
                                {
                                  // 残り時間10秒切ったら焦らす
                                  sprintf(text, "0'%05.2f", remaining_time);
                                }
                                else
                                {
                                  int time = std::floor(remaining_time);
                                  int minutes = time / 60;
                                  int seconds = time % 60;
                                  sprintf(text, "%d'%02d", minutes, seconds);
                                }
                                canvas_.setWidgetText("time_remain", text);

                                // 時間が11秒切ったら色を変える
                                auto color = (remaining_time < 11.0) ? ci::Color(1, 0, 0)
                                                                     : ci::Color(1, 1, 1);
                                canvas_.setWidgetParam("time_remain",      "color", color);
                                canvas_.setWidgetParam("time_remain_icon", "color", color);
                              });

    holder_ += event_.connect("Game:UpdateScores",
                              [this](const Connection&, const Arguments& args) noexcept
                              {
                                const auto& scores = boost::any_cast<std::vector<u_int>>(args.at("scores"));
                                updateScores(scores);
                              });


    // ゲーム完了
    holder_ += event_.connect("Game:Finish",
                              [this](const Connection&, const Arguments&) noexcept
                              {
                                canvas_.active(false);
                                canvas_.startTween("end");
                                count_exec_.add(3.0,
                                                [this]() noexcept
                                                {
                                                  active_ = false;
                                                });
                              });

    // パネル位置
    holder_ += event_.connect("Game:PutBegin",
                              [this](const Connection&, const Arguments& args) noexcept
                              {
                                {
                                  const auto& pos = boost::any_cast<glm::vec3>(args.at("pos"));
                                  auto offset = canvas_.ndcToPos(pos);
                                  
                                  const auto& widget = canvas_.at("put_timer");
                                  widget->setParam("offset", offset);
                                  widget->enable();
                                }
                                canvas_.setWidgetParam("put_timer:body", "scale", glm::vec2());
                              });
    holder_ += event_.connect("Game:PutEnd",
                              [this](const Connection&, const Arguments&) noexcept
                              {
                                canvas_.enableWidget("put_timer", false);
                              });
    holder_ += event_.connect("Game:PutHold",
                              [this](const Connection&, const Arguments& args) noexcept
                              {
                                {
                                  const auto& pos = boost::any_cast<glm::vec3>(args.at("pos"));
                                  auto offset = canvas_.ndcToPos(pos);
                                  canvas_.setWidgetParam("put_timer", "offset", offset);
                                }
                                auto scale = boost::any_cast<float>(args.at("scale"));
                                auto alpha = getEaseFunc("OutExpo")(scale);
                                canvas_.setWidgetParam("put_timer:fringe", "alpha", alpha);
                                canvas_.setWidgetParam("put_timer:body", "scale", glm::vec2(scale));
                                canvas_.setWidgetParam("put_timer:body", "alpha", alpha);
                              });

    setupCommonTweens(event_, holder_, canvas_, "pause");
    setupCommonTweens(event_, holder_, canvas_, "resume");
    setupCommonTweens(event_, holder_, canvas_, "abort");

    canvas_.startTween("start");
  }

  ~GameMain() = default;


private:
  bool update(double current_time, double delta_time) noexcept override
  {
    count_exec_.update(delta_time);
    return active_;
  }

  void updateScores(const std::vector<u_int>& scores) noexcept
  {
    int i = 1;
    for (auto score : scores)
    {
      char id[16];
      std::sprintf(id, "score:%d", i);
      canvas_.setWidgetParam(id, "text", std::to_string(score));

      i += 1;
    }
  }

};

}
