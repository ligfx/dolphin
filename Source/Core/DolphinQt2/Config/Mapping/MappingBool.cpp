// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "DolphinQt2/Config/Mapping/MappingBool.h"

#include "DolphinQt2/Config/Mapping/EmulatedControllerModel.h"
#include "InputCommon/ControllerEmu/Setting/BooleanSetting.h"

MappingBool::MappingBool(EmulatedControllerModel* model, ControllerEmu::BooleanSetting* setting)
    : QCheckBox(QString::fromStdString(setting->m_ui_name)), m_model(model), m_setting(setting)
{
  Update();
  Connect();
}

void MappingBool::Connect()
{
  connect(this, &QCheckBox::stateChanged, this, [this](int value) {
    m_setting->SetValue(value);
    m_model->SaveSettings();
  });
}

void MappingBool::Update()
{
  setChecked(m_setting->GetValue());
}
