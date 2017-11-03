// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include "DolphinQt2/QtUtils/ElidedButton.h"

class ControlReference;
class EmulatedControllerModel;
class QMouseEvent;

class MappingButton : public ElidedButton
{
  Q_OBJECT
public:
  MappingButton(EmulatedControllerModel* model, ControlReference* ref);

  void Update();

signals:
  void AdvancedPressed();

private:
  void mouseReleaseEvent(QMouseEvent* event) override;

  void Clear();
  void OnButtonPressed();
  void OnButtonTimeout();
  void Connect();

  EmulatedControllerModel* m_model;
  ControlReference* m_reference;
};
