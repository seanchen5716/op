#include <stdio.h>
#include <string.h>
#include <math.h>
#include <map>
#include "common/util.h"
#include "paint.hpp"
#include "sidebar.hpp"

static void draw_background(UIState *s) {
#ifdef QCOM
  const NVGcolor color = COLOR_BLACK_ALPHA(85);
#else
  const NVGcolor color = nvgRGBA(0x39, 0x39, 0x39, 0xff);
#endif
  ui_fill_rect(s->vg, {0, 0, sbr_w, s->fb_h}, color);
}

static void draw_settings_button(UIState *s) {
  const float alpha = s->active_app == cereal::UiLayoutState::App::SETTINGS ? 1.0f : 0.65f;
  ui_draw_image(s, settings_btn, "button_settings", alpha);
}

static void draw_home_button(UIState *s) {
  const float alpha = s->active_app == cereal::UiLayoutState::App::HOME ? 1.0f : 0.65f;
  ui_draw_image(s, home_btn, "button_home", alpha);
}

static void draw_sidebar_ipaddress(UIState *s) {
  const int ipaddress_x = 50;
  const int ipaddress_y = 210;
  const int ipaddress_w = 250;
  nvgFillColor(s->vg, COLOR_WHITE);
  nvgFontSize(s->vg, 35);
  nvgFontFace(s->vg, "sans-bold");
  nvgTextBox(s->vg, ipaddress_x, ipaddress_y, ipaddress_w, s->scene.deviceState.getWifiIpAddress().cStr(), NULL);
}

static void draw_sidebar_battery_icon(UIState *s) {
  const char *battery_img = s->scene.deviceState.getBatteryStatus() == "Charging" ? "battery_charging" : "battery";
  const Rect rect = {50, 245, 220, 65};
  ui_fill_rect(s->vg, {rect.x + 6, rect.y + 5,
              int((rect.w - 19) * s->scene.deviceState.getBatteryPercent() * 0.01), rect.h - 11}, COLOR_WHITE);
  ui_draw_image(s, rect, battery_img, 1.0f);
}

static void draw_sidebar_battery_per(UIState *s) {
  const int battery_per_x = 100;
  const int battery_per_y = 277;
  const int battery_per_w = 100;

  char battery_str[5];
  snprintf(battery_str, sizeof(battery_str), "%d%%", s->scene.deviceState.getBatteryPercent());
  nvgFillColor(s->vg, COLOR_BLACK);
  nvgFontSize(s->vg, 35);
  nvgFontFace(s->vg, "sans-bold");
  nvgTextAlign(s->vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
  nvgTextBox(s->vg, battery_per_x, battery_per_y, battery_per_w, battery_str, NULL);
}

/*
static void draw_network_type(UIState *s) {
  static std::map<cereal::DeviceState::NetworkType, const char *> network_type_map = {
      {cereal::DeviceState::NetworkType::NONE, "--"},
      {cereal::DeviceState::NetworkType::WIFI, "WiFi"},
      {cereal::DeviceState::NetworkType::CELL2_G, "2G"},
      {cereal::DeviceState::NetworkType::CELL3_G, "3G"},
      {cereal::DeviceState::NetworkType::CELL4_G, "4G"},
      {cereal::DeviceState::NetworkType::CELL5_G, "5G"}};
  const int network_x = 50;
  const int network_y = 273;
  const int network_w = 100;
  const char *network_type = network_type_map[s->scene.deviceState.getNetworkType()];
  nvgFillColor(s->vg, COLOR_WHITE);
  nvgFontSize(s->vg, 48);
  nvgFontFace(s->vg, "sans-regular");
  nvgTextAlign(s->vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
  nvgTextBox(s->vg, network_x, network_y, network_w, network_type ? network_type : "--", NULL);
}
*/

static void draw_metric(UIState *s, const char *label_str, const char *value_str, const int severity, const int y_offset, const char *message_str) {
  NVGcolor status_color;

  if (severity == 0) {
    status_color = COLOR_WHITE;
  } else if (severity == 1) {
    status_color = COLOR_YELLOW;
  } else if (severity > 1) {
    status_color = COLOR_RED;
  }

  const Rect rect = {30, 338 + y_offset, 240, message_str ? strchr(message_str, '\n') ? 124 : 100 : 148};
  ui_draw_rect(s->vg, rect, severity > 0 ? COLOR_WHITE : COLOR_WHITE_ALPHA(85), 2, 20.);

  nvgBeginPath(s->vg);
  nvgRoundedRectVarying(s->vg, rect.x + 6, rect.y + 6, 18, rect.h - 12, 25, 0, 0, 25);
  nvgFillColor(s->vg, status_color);
  nvgFill(s->vg);

  if (!message_str) {
    nvgFillColor(s->vg, COLOR_WHITE);
    nvgFontSize(s->vg, 78);
    nvgFontFace(s->vg, "sans-bold");
    nvgTextAlign(s->vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgTextBox(s->vg, rect.x + 50, rect.y + 50, rect.w - 60, value_str, NULL);

    nvgFillColor(s->vg, COLOR_WHITE);
    nvgFontSize(s->vg, 48);
    nvgFontFace(s->vg, "sans-bold");
    nvgTextAlign(s->vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgTextBox(s->vg, rect.x + 50, rect.y + 50 + 66, rect.w - 60, label_str, NULL);
  } else {
    nvgFillColor(s->vg, COLOR_WHITE);
    nvgFontSize(s->vg, 48);
    nvgFontFace(s->vg, "sans-bold");
    nvgTextAlign(s->vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgTextBox(s->vg, rect.x + 35, rect.y + (strchr(message_str, '\n') ? 40 : 50), rect.w - 50, message_str, NULL);
  }
}

static void draw_temp_metric(UIState *s) {
  static std::map<cereal::DeviceState::ThermalStatus, const int> temp_severity_map = {
      {cereal::DeviceState::ThermalStatus::GREEN, 0},
      {cereal::DeviceState::ThermalStatus::YELLOW, 1},
      {cereal::DeviceState::ThermalStatus::RED, 2},
      {cereal::DeviceState::ThermalStatus::DANGER, 3}};
  std::string temp_val = std::to_string((int)s->scene.deviceState.getAmbientTempC()) + "°C";
  draw_metric(s, "장치 온도", temp_val.c_str(), temp_severity_map[s->scene.deviceState.getThermalStatus()], 0, NULL);
}

static void draw_panda_metric(UIState *s) {
  const int panda_y_offset = 32 + 148;

  int panda_severity = 0;
  std::string panda_message = "판다\n연결됨";
  if (s->scene.pandaType == cereal::PandaState::PandaType::UNKNOWN) {
    panda_severity = 2;
    panda_message = "판다\n연결안됨";
  }
#ifdef QCOM2
  else if (s->started) {
    panda_severity = s->scene.gpsOK ? 0 : 1;
    panda_message = util::string_format("SAT CNT\n%d", s->scene.satelliteCount);
  }
#endif

  draw_metric(s, NULL, NULL, panda_severity, panda_y_offset, panda_message.c_str());
}

static void draw_connectivity(UIState *s) {
  static std::map<NetStatus, std::pair<const char *, int>> connectivity_map = {
      {NET_ERROR, {"네트워크\n에러", 2}},
      {NET_CONNECTED, {"네트워크\n온라인", 0}},
      {NET_DISCONNECTED, {"네트워크\n오프라인", 1}},
  };
  auto net_params = connectivity_map[s->scene.athenaStatus];
  draw_metric(s, NULL, NULL, net_params.second, 180 + 158, net_params.first);
}

void ui_draw_sidebar(UIState *s) {
  if (s->sidebar_collapsed) {
    return;
  }
  draw_background(s);
  draw_settings_button(s);
  draw_home_button(s);
  draw_sidebar_ipaddress(s);
  draw_sidebar_battery_icon(s);
  draw_sidebar_battery_per(s);
//  draw_network_type(s);
  draw_temp_metric(s);
  draw_panda_metric(s);
  draw_connectivity(s);
}
