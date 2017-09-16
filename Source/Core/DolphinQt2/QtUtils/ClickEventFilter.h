// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <QObject>

class ClickEventFilter : public QObject
{
  Q_OBJECT
signals:
  void clicked();
  void middleClicked();
  void rightClicked();
  void doubleClicked();

private:
  bool eventFilter(QObject* object, QEvent* event) override;
};
