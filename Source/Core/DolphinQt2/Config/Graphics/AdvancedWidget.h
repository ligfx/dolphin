// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <QWidget>
#include <functional>

class QCheckBox;

class AdvancedWidget final : public QWidget
{
  Q_OBJECT
public:
  explicit AdvancedWidget(QWidget* parent = nullptr);
  void ForEachDescription(std::function<void(QWidget*, const char*)> f);

private:
  void LoadSettings();
  void SaveSettings();

  void CreateWidgets();
  void ConnectWidgets();
  void OnBackendChanged();
  void OnEmulationStateChanged(bool running);

  // Debugging
  QCheckBox* m_enable_wireframe;
  QCheckBox* m_show_statistics;
  QCheckBox* m_enable_format_overlay;
  QCheckBox* m_enable_api_validation;

  // Utility
  QCheckBox* m_dump_textures;
  QCheckBox* m_prefetch_custom_textures;
  QCheckBox* m_dump_efb_target;
  QCheckBox* m_dump_use_ffv1;
  QCheckBox* m_load_custom_textures;
  QCheckBox* m_use_fullres_framedumps;
  QCheckBox* m_enable_freelook;

  // Misc
  QCheckBox* m_enable_cropping;
  QCheckBox* m_enable_prog_scan;
  QCheckBox* m_borderless_fullscreen;
};
