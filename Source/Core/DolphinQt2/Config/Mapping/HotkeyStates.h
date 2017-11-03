// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include "DolphinQt2/Config/Mapping/MappingWidget.h"

class QHBoxLayout;

class HotkeyStates final : public MappingWidget
{
public:
  explicit HotkeyStates(EmulatedControllerModel* model);

  InputConfig* GetConfig() override;

private:
  void LoadSettings() override;
  void CreateMainLayout();

  // Main
  QHBoxLayout* m_main_layout;
};
