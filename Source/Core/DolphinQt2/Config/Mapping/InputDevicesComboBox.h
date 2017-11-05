// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <QWidget>

class QComboBox;

namespace ciface
{
namespace Core
{
class DeviceQualifier;
};
};

class InputDevicesComboBox : public QWidget
{
  Q_OBJECT
public:
  InputDevicesComboBox();
  std::string GetDeviceString() const;
  ciface::Core::DeviceQualifier GetDeviceQualifier() const;
  void SetDevice(const std::string& device);
  void SetDevice(const ciface::Core::DeviceQualifier& devq);

signals:
  void DeviceChanged(const std::string& device);

private:
  QComboBox* m_combo;
};
