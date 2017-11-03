#include "DolphinQt2/Config/Mapping/EmulatedControllerModel.h"

#include "InputCommon/ControllerEmu/ControllerEmu.h"
#include "InputCommon/ControllerInterface/ControllerInterface.h"

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
