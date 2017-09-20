// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "DolphinQt2/QtUtils/Bind.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QObject>

void Bind(QCheckBox* checkbox, bool& value)
{
  checkbox->setChecked(value);
  QObject::connect(checkbox, &QCheckBox::toggled, [&value](bool checked) { value = checked; });
}

void Bind(QButtonGroup* button_group, int& value)
{
  if (button_group->button(value))
    button_group->button(value)->setChecked(true);

  QObject::connect(button_group,
                   static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
                   [&value](int id) { value = id; });
}
