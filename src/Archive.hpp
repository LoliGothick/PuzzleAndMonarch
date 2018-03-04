﻿#pragma once

//
// ゲーム内記録
//

#include <boost/noncopyable.hpp>
#include "Score.hpp"


namespace ngs {

class Archive
  : private boost::noncopyable
{
  void create() noexcept
  {
    records_.addChild(ci::JsonTree("play-times",         uint32_t(0)))
            .addChild(ci::JsonTree("high-score",         uint32_t(0)))
            .addChild(ci::JsonTree("total-panels",       uint32_t(0)))
            .addChild(ci::JsonTree("panel-turned-times", uint32_t(0)))
            .addChild(ci::JsonTree("panel-moved-times",  uint32_t(0)))
            .addChild(ci::JsonTree("share-times",        uint32_t(0)))
            .addChild(ci::JsonTree("startup-times",      uint32_t(0)))
            .addChild(ci::JsonTree("abort-times",        uint32_t(0)))

            .addChild(ci::JsonTree("average-score",       0.0))
            .addChild(ci::JsonTree("average-put-panels",  0.0))
            .addChild(ci::JsonTree("average-moved-times", 0.0))
            .addChild(ci::JsonTree("average-turn-times",  0.0))
            .addChild(ci::JsonTree("average-put-time",    0.0))

            .addChild(ci::JsonTree("bgm-enable", true))
            .addChild(ci::JsonTree("se-enable",  true))

            .addChild(ci::JsonTree::makeArray("games"))

            .addChild(ci::JsonTree("version", version_))
    ;
  }

  void load() noexcept
  {
    if (!ci::fs::is_regular_file(full_path_))
    {
      // 記録ファイルが無い
      create();
      DOUT << "Archive:create: " << full_path_ << std::endl;
      return;
    }

    records_ = ci::JsonTree(ci::loadFile(full_path_));
    DOUT << "Archive:load: " << full_path_ << std::endl;
  }


public:
  Archive(const std::string& path, const std::string& version) noexcept
    : full_path_(getDocumentPath() / path),
      version_(version)
  {
    load();
  }

  ~Archive() = default;


  // Gameの記録が保存されているか？
  bool isSaved() const noexcept
  {
    const auto& json = getRecordArray("games");
    return json.hasChildren();
  }


  // プレイ結果を記録
  void recordGameResults(const Score& score) noexcept
  {
    // 累積記録
    addRecord("play-times",   uint32_t(1));
    addRecord("total-panels", uint32_t(score.total_panels));
    addRecord("panel-turned-times", uint32_t(score.panel_turned_times));
    addRecord("panel-moved-times",  uint32_t(score.panel_moved_times));

    if (score.high_score)
    {
      setRecord("high-score", uint32_t(score.total_score));
    }

    // 平均値などを計算
    auto play_times = getRecord<uint32_t>("play-times");
    {
      auto average = getRecord<double>("average-score");
      average = (average * (play_times - 1) + score.total_score) / play_times;
      setRecord("average-score", average);
    }
    {
      auto average = getRecord<double>("average-put-panels");
      average = (average * (play_times - 1) + score.total_panels) / play_times;
      setRecord("average-put-panels", average);
    }
    {
      auto average = getRecord<double>("average-moved-times");
      average = (average * (play_times - 1) + score.panel_moved_times) / play_times;
      setRecord("average-moved-times", average);
    }
    {
      auto average = getRecord<double>("average-turn-times");
      average = (average * (play_times - 1) + score.panel_turned_times) / play_times;
      setRecord("average-turn-times", average);
    }
    {
      auto average = getRecord<double>("average-put-time");

      // NOTICE １つも置けなかった場合は１つ置いた時と同じ扱い
      auto panels = std::max(score.total_panels, u_int(1));
      auto put_time = score.limit_time / double(panels);
      average = (average * (play_times - 1) + put_time) / play_times;
      setRecord("average-put-time", average);
    }

    // 記録→保存
    save();
  }

  
  // 記録を取得
  template <typename T>
  T getRecord(const std::string& id) const noexcept
  {
    return records_.getValueForKey<T>(id);
  }

  // 記録を変更
  template <typename T>
  void setRecord(const std::string& id, const T& value) noexcept
  {
    records_[id] = ci::JsonTree(id, value);
  }

  // 値に加算する
  template <typename T>
  void addRecord(const std::string& id, T value) noexcept
  {
    auto v = getRecord<T>(id);
    setRecord(id, T(v + value));
  }

  void setRecordArray(const std::string&id, const ci::JsonTree& json) noexcept
  {
    records_[id] = json;
  }

  const ci::JsonTree& getRecordArray(const std::string& id) const noexcept
  {
    return records_[id];
  }


  void save() noexcept
  {
    records_.write(full_path_);
    DOUT << "Archive:write: " << full_path_ << std::endl;
  }


private:
  std::string version_;

  ci::fs::path full_path_;

  ci::JsonTree records_;
};

}
