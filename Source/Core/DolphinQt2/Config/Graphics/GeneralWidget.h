// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <QWidget>
#include <functional>

class QCheckBox;
class QComboBox;
class QGridLayout;

namespace X11Utils
{
class XRRConfiguration;
}

class GeneralWidget final : public QWidget
{
  Q_OBJECT
public:
  explicit GeneralWidget(X11Utils::XRRConfiguration* xrr_config, QWidget* parent = nullptr);
  void ForEachDescription(std::function<void(QWidget*, const char*)> f);

private:
  void LoadSettings();
  void SaveSettings();
  void SaveBackendSetting();

  void CreateWidgets();
  void ConnectWidgets();

  void OnBackendChanged(const std::string& backend_name);
  void OnEmulationStateChanged(bool running);

  // Video
  QGridLayout* m_video_layout;
  QComboBox* m_backend_combo;
  QComboBox* m_adapter_combo;
  QComboBox* m_aspect_combo;
  QCheckBox* m_enable_vsync;
  QCheckBox* m_enable_fullscreen;

  // Options
  QCheckBox* m_show_fps;
  QCheckBox* m_show_ping;
  QCheckBox* m_log_render_time;
  QCheckBox* m_autoadjust_window_size;
  QCheckBox* m_show_messages;
  QCheckBox* m_keep_window_top;
  QCheckBox* m_hide_cursor;
  QCheckBox* m_render_main_window;

  X11Utils::XRRConfiguration* m_xrr_config;
};
