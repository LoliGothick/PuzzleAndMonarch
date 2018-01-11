﻿#pragma once

//
// UI Test
//

#include <cinder/Camera.h>
#include <cinder/gl/GlslProg.h>
#include "Font.hpp"
#include "Shader.hpp"
#include "UIWidget.hpp"
#include "UIWidgetsFactory.hpp"


namespace ngs {

class UITest {



public:
  UITest(const ci::JsonTree& params) noexcept
    : fov(params.getValueForKey<float>("ui_camera.fov")),
      near_z(params.getValueForKey<float>("ui_camera.near_z")),
      far_z(params.getValueForKey<float>("ui_camera.far_z")),
      camera(ci::app::getWindowWidth(), ci::app::getWindowHeight(), fov, near_z, far_z),
      font("DFKAIC001.ttf"),
      shader(createShader("font", "font"))
  {
    camera.lookAt(Json::getVec<glm::vec3>(params["ui_camera.eye"]), Json::getVec<glm::vec3>(params["ui_camera.target"]));
    font.size(params.getValueForKey<float>("ui_test.font_size"));

    widgets_ = widgets_factory_.construct(params["ui_test.widgets"]);
  }


  void resize(float aspect) noexcept
  {
    camera.setAspectRatio(aspect);
    if (aspect < 1.0) {
      // 画面が縦長になったら、幅基準でfovを求める
      // fovとnear_zから投影面の幅の半分を求める
      float half_w = std::tan(ci::toRadians(fov / 2)) * near_z;

      // 表示画面の縦横比から、投影面の高さの半分を求める
      float half_h = half_w / aspect;

      // 投影面の高さの半分とnear_zから、fovが求まる
      float fov_w = std::atan(half_h / near_z) * 2;
      camera.setFov(ci::toDegrees(fov_w));
    }
    else {
      // 横長の場合、fovは固定
      camera.setFov(fov);
    }
  }


  void update() noexcept
  {
  }

  void draw(glm::ivec2 window_size) noexcept
  {
    ci::gl::enableDepth(false);
    ci::gl::disable(GL_CULL_FACE);
    ci::gl::enableAlphaBlending();
    ci::gl::setMatrices(camera);
    ci::gl::ScopedGlslProg prog(shader);

  }


private:
  float fov;
  float near_z;
  float far_z;

  ci::CameraPersp camera;
  Font font;
  ci::gl::GlslProgRef shader;

  UI::WidgetPtr widgets_; 
  UI::WidgetsFactory widgets_factory_;
};

}
