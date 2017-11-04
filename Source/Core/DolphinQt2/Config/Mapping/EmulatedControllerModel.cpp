#include "DolphinQt2/Config/Mapping/EmulatedControllerModel.h"

#include "Common/FileUtil.h"
#include "Common/IniFile.h"
#include "DolphinQt2/Settings.h"
#include "InputCommon/ControlReference/ControlReference.h"
#include "InputCommon/ControllerEmu/Control/Control.h"
#include "InputCommon/ControllerEmu/ControlGroup/ControlGroup.h"
#include "InputCommon/ControllerEmu/ControllerEmu.h"
#include "InputCommon/ControllerEmu/Setting/BooleanSetting.h"
#include "InputCommon/ControllerEmu/Setting/NumericSetting.h"
#include "InputCommon/ControllerInterface/ControllerInterface.h"
#include "InputCommon/InputConfig.h"

EmulatedControllerModel::EmulatedControllerModel()
{
  connect(&Settings::Instance(), &Settings::DevicesChanged, this, [this] {
    if (!m_controller)
      return;
    m_controller->UpdateReferences(g_controller_interface);
  });
}

void EmulatedControllerModel::SaveProfile(const std::string& profile_path)
{
  File::CreateFullPath(profile_path);

  IniFile ini;
  m_controller->SaveConfig(ini.GetOrCreateSection("Profile"));
  ini.Save(profile_path);
}

void EmulatedControllerModel::LoadProfile(const std::string& profile_path)
{
  IniFile ini;
  ini.Load(profile_path);

  m_controller->LoadConfig(ini.GetOrCreateSection("Profile"));
  m_controller->UpdateReferences(g_controller_interface);

  emit DefaultDeviceChanged();
  emit Update();
}

void EmulatedControllerModel::LoadDefaults()
{
  if (m_controller == nullptr)
    return;

  m_controller->LoadDefaults(g_controller_interface);
  m_controller->UpdateReferences(g_controller_interface);
  emit DefaultDeviceChanged();
  emit Update();
}

void EmulatedControllerModel::SetDevice(const std::string& device)
{
  ciface::Core::DeviceQualifier devq;
  devq.FromString(device);
  if (devq == m_controller->GetDefaultDevice())
    return;

  m_controller->SetDefaultDevice(std::move(devq));
  emit DefaultDeviceChanged();
}

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

InputConfig* EmulatedControllerModel::GetConfig() const
{
  return m_config;
}

void EmulatedControllerModel::SetConfig(InputConfig* config)
{
  m_config = config;
  m_controller = m_config->GetController(m_port);
}

void EmulatedControllerModel::SaveSettings()
{
  GetConfig()->SaveConfig();
}
