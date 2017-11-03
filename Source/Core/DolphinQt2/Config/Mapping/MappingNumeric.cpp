// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "DolphinQt2/Config/Mapping/MappingNumeric.h"

#include "DolphinQt2/Config/Mapping/EmulatedControllerModel.h"
#include "InputCommon/ControllerEmu/Setting/NumericSetting.h"

MappingNumeric::MappingNumeric(EmulatedControllerModel* model,
                               ControllerEmu::NumericSetting* setting)
    : m_model(model), m_setting(setting), m_range(setting->m_high - setting->m_low)
{
  setRange(setting->m_low, setting->m_high);
  Update();
  Connect();
}

void MappingNumeric::Connect()
{
  connect(this, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
          [this](int value) {
            m_setting->SetValue(static_cast<double>(value - m_setting->m_low) / m_range);
            m_model->SaveSettings();
          });
}

void MappingNumeric::Update()
{
  setValue(m_setting->m_low + m_setting->GetValue() * m_range);
}
