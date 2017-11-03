// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <QCheckBox>

class EmulatedControllerModel;

namespace ControllerEmu
{
class BooleanSetting;
};

class MappingBool : public QCheckBox
{
public:
  MappingBool(EmulatedControllerModel* model, ControllerEmu::BooleanSetting* setting);

  void Clear();
  void Update();

private:
  void Connect();

  EmulatedControllerModel* m_model;
  ControllerEmu::BooleanSetting* m_setting;
};
