// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "DolphinQt2/Config/Mapping/InputDevicesComboBox.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QSignalBlocker>

#include "DolphinQt2/Config/Mapping/EmulatedControllerModel.h"
#include "DolphinQt2/Settings.h"
#include "InputCommon/ControllerEmu/ControllerEmu.h"
#include "InputCommon/ControllerInterface/ControllerInterface.h"
#include "InputCommon/ControllerInterface/Device.h"

InputDevicesComboBox::InputDevicesComboBox()
{
  auto* layout = new QHBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout);

  m_combo = new QComboBox();
  m_combo->setEditable(true);
  layout->addWidget(m_combo);

  // ConnectWidgets
  connect(m_combo, &QComboBox::editTextChanged, this,
          [this] { emit DeviceChanged(GetDeviceString()); });

  connect(&Settings::Instance(), &Settings::DevicesChanged, this,
          [this] { SetDevice(GetDeviceString()); });
}

std::string InputDevicesComboBox::GetDeviceString() const
{
  return m_combo->currentText().toStdString();
}

ciface::Core::DeviceQualifier InputDevicesComboBox::GetDeviceQualifier() const
{
  ciface::Core::DeviceQualifier devq;
  devq.FromString(GetDeviceString());
  return devq;
}

void InputDevicesComboBox::SetDevice(const std::string& device)
{
  const QSignalBlocker blocker(m_combo);
  m_combo->clear();

  m_combo->addItem(QString::fromStdString(device));
  m_combo->setCurrentIndex(0);

  for (const auto& name : g_controller_interface.GetAllDeviceStrings())
  {
    if (name != device)
      m_combo->addItem(QString::fromStdString(name));
  }

  emit DeviceChanged(device);
}

void InputDevicesComboBox::SetDevice(const ciface::Core::DeviceQualifier& devq)
{
  SetDevice(devq.ToString());
}
