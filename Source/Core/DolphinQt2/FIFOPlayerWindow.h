// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <QDialog>

class QCheckBox;
class QDialogButtonBox;
class QLabel;
class QPushButton;
class QSpinBox;
class SpinBoxRangePair;

class FIFOPlayerWindow : public QDialog
{
  Q_OBJECT
public:
  explicit FIFOPlayerWindow(QWidget* parent = nullptr);
  ~FIFOPlayerWindow();

signals:
  void LoadFIFORequested(const QString& path);

private:
  void CreateWidgets();
  void ConnectWidgets();

  void LoadRecording();
  void SaveRecording();
  void StartRecording();
  void StopRecording();

  void OnFIFOLoaded();

  void Update();

  QLabel* m_info_label;
  QPushButton* m_load;
  QPushButton* m_save;
  QPushButton* m_record;
  QPushButton* m_stop;
  SpinBoxRangePair* m_frame_range;
  QSpinBox* m_frame_record_count;
  QLabel* m_frame_record_count_label;
  SpinBoxRangePair* m_object_range;
  QCheckBox* m_early_memory_updates;
  QDialogButtonBox* m_button_box;
};
