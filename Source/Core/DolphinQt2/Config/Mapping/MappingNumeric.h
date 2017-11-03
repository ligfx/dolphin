// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <QSpinBox>

class EmulatedControllerModel;

namespace ControllerEmu
{
class NumericSetting;
}

class MappingNumeric : public QSpinBox
{
public:
  MappingNumeric(EmulatedControllerModel* model, ControllerEmu::NumericSetting* ref);

  void Clear();
  void Update();

private:
  void Connect();

  EmulatedControllerModel* m_model;
  ControllerEmu::NumericSetting* m_setting;
  double m_range;
};
