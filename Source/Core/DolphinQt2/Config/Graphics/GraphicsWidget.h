// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <QWidget>

class GraphicsWindow;
class QFormLayout;
class QGroupBox;
class QLabel;

class GraphicsWidget : public QWidget
{
  Q_OBJECT
public:
  explicit GraphicsWidget(GraphicsWindow* parent);

protected:
  void AddWidget(const QString& name, QWidget* widget);
  void AddWidget(QWidget* widget);

  virtual void LoadSettings() = 0;
  virtual void SaveSettings() = 0;

private:
  QFormLayout* m_main_layout;
};
