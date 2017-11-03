#include "DolphinQt2/Config/Mapping/EmulatedControllerModel.h"

#include "InputCommon/ControlReference/ControlReference.h"
#include "InputCommon/ControllerEmu/Control/Control.h"
#include "InputCommon/ControllerEmu/ControlGroup/ControlGroup.h"
#include "InputCommon/ControllerEmu/ControllerEmu.h"
#include "InputCommon/ControllerEmu/Setting/BooleanSetting.h"
#include "InputCommon/ControllerEmu/Setting/NumericSetting.h"
#include "InputCommon/ControllerInterface/ControllerInterface.h"

void EmulatedControllerModel::Clear()
{
  if (!m_controller)
    return;

  for (auto& group : m_controller->groups)
  {
    for (auto& control : group->controls)
      control->control_ref->SetExpression("");

    for (auto& numeric_setting : group->numeric_settings)
      numeric_setting->m_value =
          numeric_setting->m_low + (numeric_setting->m_low + numeric_setting->m_high) / 2;

    for (auto& boolean_setting : group->boolean_settings)
      boolean_setting->m_value = false;
  }

  Update();
}

int EmulatedControllerModel::GetPort() const
{
  return m_port;
}

ControllerEmu::EmulatedController* EmulatedControllerModel::GetController() const
{
  return m_controller;
}

std::shared_ptr<ciface::Core::Device> EmulatedControllerModel::GetDevice() const
{
  return g_controller_interface.FindDevice(GetController()->GetDefaultDevice());
}
