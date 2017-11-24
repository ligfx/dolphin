// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <QWidget>

class QLabel;
class QSpinBox;
class QString;

class SpinBoxRangePair : public QWidget
{
  Q_OBJECT
public:
  SpinBoxRangePair(const QString& first_text, const QString& second_text);

  int GetRangeStart() const;
  int GetRangeEnd() const;
  void SetRangeEnd(int range_end);
  void SetMaximumRangeEnd(int maximum);
  void SetEnabled(bool enabled);

signals:
  void ValuesChanged(int first_value, int second_value);

private:
  void Update();

  QLabel* m_first_label;
  QSpinBox* m_first_spinbox;
  QLabel* m_second_label;
  QSpinBox* m_second_spinbox;
};
