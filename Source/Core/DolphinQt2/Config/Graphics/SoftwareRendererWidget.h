// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <QWidget>
#include <functional>

class GraphicsWindow;
class QCheckBox;
class QComboBox;
class QSpinBox;

class SoftwareRendererWidget final : public QWidget
{
  Q_OBJECT
public:
  explicit SoftwareRendererWidget(QWidget* parent = nullptr);
  void ForEachDescription(std::function<void(QWidget*, const char*)> f);

private:
  void LoadSettings();
  void SaveSettings();

  void CreateWidgets();
  void ConnectWidgets();

  QComboBox* m_backend_combo;
  QCheckBox* m_disable_xfb;
  QCheckBox* m_enable_statistics;
  QCheckBox* m_dump_textures;
  QCheckBox* m_dump_objects;
  QCheckBox* m_dump_tev_stages;
  QCheckBox* m_dump_tev_fetches;

  QSpinBox* m_object_range_min;
  QSpinBox* m_object_range_max;
};
