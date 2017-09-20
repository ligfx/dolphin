// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <QComboBox>
#include <QObject>

class QButtonGroup;
class QCheckBox;

void Bind(QCheckBox* checkbox, bool& value);
void Bind(QButtonGroup* button_group, int& value);

template <typename T>
void Bind(QComboBox* combobox, T& value)
{
  int index = combobox->findData(QVariant::fromValue(value));
  if (index != -1)
    combobox->setCurrentIndex(index);

  QObject::connect(combobox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                   [combobox, &value](int index) {
                     if (index == -1)
                       return;
                     value = combobox->itemData(index).value<T>();
                   });
}
