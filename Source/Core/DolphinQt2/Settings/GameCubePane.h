// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <QWidget>
#include <array>

class QCheckBox;
class QPushButton;
class QComboBox;
class QLabel;

class GameCubePane final : public QWidget
{
  Q_OBJECT
public:
  explicit GameCubePane(QWidget* parent = nullptr);

private:
  void CreateLayout();
  void ConnectLayout();

  QCheckBox* m_skip_ipl_checkbox;
  QLabel* m_system_language_label;
  QComboBox* m_system_language_combo;
  QCheckBox* m_override_language_checkbox;

  std::array<QComboBox*, 3> m_exi_device_combos;
  std::array<QPushButton*, 2> m_exi_device_buttons;
};
