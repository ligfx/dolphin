// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include <QEvent>
#include <QMouseEvent>

#include "DolphinQt2/QtUtils/ClickEventFilter.h"

bool ClickEventFilter::eventFilter(QObject* object, QEvent* event)
{
  if (event->type() == QEvent::MouseButtonDblClick)
    emit doubleClicked();

  if (event->type() == QEvent::MouseButtonRelease)
  {
    switch (static_cast<QMouseEvent*>(event)->button())
    {
    case Qt::MouseButton::LeftButton:
      emit clicked();
      break;
    case Qt::MouseButton::MiddleButton:
      emit middleClicked();
      break;
    case Qt::MouseButton::RightButton:
      emit rightClicked();
      break;
    default:
      break;
    }
  }

  return false;
}
