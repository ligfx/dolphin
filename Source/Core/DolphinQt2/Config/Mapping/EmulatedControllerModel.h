// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <QObject>
#include <memory>

#include "InputCommon/ControllerInterface/Device.h"

namespace ControllerEmu
{
class EmulatedController;
}

class EmulatedControllerModel : public QObject
{
  Q_OBJECT
public:
  void Clear();
  int GetPort() const;
  std::shared_ptr<ciface::Core::Device> GetDevice() const;
  ControllerEmu::EmulatedController* GetController() const;

  ControllerEmu::EmulatedController* m_controller = nullptr;
  int m_port = -1;

signals:
  void Update();
};
