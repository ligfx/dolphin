// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include <vector>

#include <QString>
#include <QWidget>

class ControlGroupBox;
class InputConfig;
class IOWindow;
class MappingBool;
class MappingButton;
class MappingNumeric;
class MappingWindow;
class QGroupBox;

namespace ControllerEmu
{
class Control;
class ControlGroup;
class EmulatedController;
}

namespace ciface
{
namespace Core
{
class Device;
}
}

class EmulatedControllerModel;

class MappingWidget : public QWidget
{
  Q_OBJECT
public:
  explicit MappingWidget(EmulatedControllerModel* model);

  ControllerEmu::EmulatedController* GetController() const;
  std::shared_ptr<ciface::Core::Device> GetDevice() const;

  virtual void LoadSettings() = 0;
  void SaveSettings();
  virtual InputConfig* GetConfig() = 0;
  EmulatedControllerModel* GetModel() const;

  void Update();

protected:
  int GetPort() const;
  QGroupBox* CreateGroupBox(const QString& name, ControllerEmu::ControlGroup* group);

private:
  void OnClearFields();

  EmulatedControllerModel* m_model;
  bool m_first = true;
  std::vector<MappingBool*> m_bools;
  std::vector<MappingButton*> m_buttons;
  std::vector<MappingNumeric*> m_numerics;
};
