// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "DolphinQt2/QtUtils/SpinBoxRangePair.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>

SpinBoxRangePair::SpinBoxRangePair(const QString& first_text, const QString& second_text)
{
  setLayout(new QHBoxLayout());
  layout()->setContentsMargins(0, 0, 0, 0);

  layout()->addWidget(m_first_label = new QLabel(first_text));
  layout()->addWidget(m_first_spinbox = new QSpinBox());
  layout()->addWidget(m_second_label = new QLabel(second_text));
  layout()->addWidget(m_second_spinbox = new QSpinBox());

  connect(m_first_spinbox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
          &SpinBoxRangePair::Update);
  connect(m_second_spinbox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
          &SpinBoxRangePair::Update);
}

int SpinBoxRangePair::GetRangeStart() const
{
  return m_first_spinbox->value();
}
int SpinBoxRangePair::GetRangeEnd() const
{
  return m_second_spinbox->value();
}
void SpinBoxRangePair::SetRangeEnd(int range_end)
{
  m_second_spinbox->setValue(range_end);
}
void SpinBoxRangePair::SetMaximumRangeEnd(int maximum)
{
  m_second_spinbox->setMaximum(maximum);
}
void SpinBoxRangePair::SetEnabled(bool enabled)
{
  m_first_label->setEnabled(enabled);
  m_first_spinbox->setEnabled(enabled);
  m_second_label->setEnabled(enabled);
  m_second_spinbox->setEnabled(enabled);
}

void SpinBoxRangePair::Update()
{
  m_first_spinbox->setMaximum(std::max(GetRangeEnd() - 1, 0));
  m_second_spinbox->setMinimum(GetRangeStart() + 1);
  emit ValuesChanged(GetRangeStart(), GetRangeEnd());
}
