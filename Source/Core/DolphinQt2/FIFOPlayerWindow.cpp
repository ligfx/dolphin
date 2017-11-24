// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "DolphinQt2/FIFOPlayerWindow.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include <algorithm>

#include "Core/Core.h"
#include "Core/FifoPlayer/FifoDataFile.h"
#include "Core/FifoPlayer/FifoPlaybackAnalyzer.h"
#include "Core/FifoPlayer/FifoPlayer.h"
#include "Core/FifoPlayer/FifoRecorder.h"

#include "DolphinQt2/QtUtils/QueueOnObject.h"
#include "DolphinQt2/QtUtils/SpinBoxRangePair.h"
#include "DolphinQt2/Settings.h"

FIFOPlayerWindow::FIFOPlayerWindow(QWidget* parent) : QDialog(parent)
{
  setWindowTitle(tr("FIFO Player"));

  CreateWidgets();
  ConnectWidgets();

  Update();

  FifoPlayer::GetInstance().SetFileLoadedCallback(
      [this] { QueueOnObject(this, &FIFOPlayerWindow::OnFIFOLoaded); });
  FifoPlayer::GetInstance().SetFrameWrittenCallback(
      [this] { QueueOnObject(this, &FIFOPlayerWindow::Update); });

  connect(&Settings::Instance(), &Settings::EmulationStateChanged, this, [this](Core::State state) {
    // If we have previously been recording, stop now.
    if (state == Core::State::Uninitialized && FifoRecorder::GetInstance().IsRecording())
      StopRecording();

    Update();
  });
}

FIFOPlayerWindow::~FIFOPlayerWindow()
{
  FifoPlayer::GetInstance().SetFileLoadedCallback({});
  FifoPlayer::GetInstance().SetFrameWrittenCallback({});
}

void FIFOPlayerWindow::CreateWidgets()
{
  auto* layout = new QVBoxLayout;

  // Info
  auto* info_group = new QGroupBox(tr("File Info"));
  auto* info_layout = new QHBoxLayout;

  m_info_label = new QLabel;
  info_layout->addWidget(m_info_label);
  info_group->setLayout(info_layout);

  m_info_label->setFixedHeight(QFontMetrics(font()).lineSpacing() * 3);

  // Object Range
  auto* object_range_group = new QGroupBox(tr("Object Range"));
  m_object_range = new SpinBoxRangePair(tr("From:"), tr("To:"));
  object_range_group->setLayout(new QHBoxLayout);
  object_range_group->layout()->addWidget(m_object_range);

  // Frame Range
  auto* frame_range_group = new QGroupBox(tr("Frame Range"));
  m_frame_range = new SpinBoxRangePair(tr("From:"), tr("To:"));
  frame_range_group->setLayout(new QHBoxLayout);
  frame_range_group->layout()->addWidget(m_frame_range);

  // Playback Options
  auto* playback_group = new QGroupBox(tr("Playback Options"));
  auto* playback_layout = new QGridLayout;
  m_early_memory_updates = new QCheckBox(tr("Early Memory Updates"));

  playback_layout->addWidget(object_range_group, 0, 0);
  playback_layout->addWidget(frame_range_group, 0, 1);
  playback_layout->addWidget(m_early_memory_updates, 1, 0, 1, -1);
  playback_group->setLayout(playback_layout);

  // Recording Options
  auto* recording_group = new QGroupBox(tr("Recording Options"));
  auto* recording_layout = new QHBoxLayout;
  m_frame_record_count = new QSpinBox;
  m_frame_record_count_label = new QLabel(tr("Frames to Record:"));

  m_frame_record_count->setMinimum(1);
  m_frame_record_count->setMaximum(3600);
  m_frame_record_count->setValue(3);

  recording_layout->addWidget(m_frame_record_count_label);
  recording_layout->addWidget(m_frame_record_count);
  recording_group->setLayout(recording_layout);

  m_button_box = new QDialogButtonBox(QDialogButtonBox::Close);

  // Action Buttons
  m_load = m_button_box->addButton(tr("Load..."), QDialogButtonBox::ActionRole);
  m_save = m_button_box->addButton(tr("Save..."), QDialogButtonBox::ActionRole);
  m_record = m_button_box->addButton(tr("Record"), QDialogButtonBox::ActionRole);
  m_stop = m_button_box->addButton(tr("Stop"), QDialogButtonBox::ActionRole);

  layout->addWidget(info_group);
  layout->addWidget(playback_group);
  layout->addWidget(recording_group);
  layout->addWidget(m_button_box);

  setLayout(layout);
}

void FIFOPlayerWindow::ConnectWidgets()
{
  connect(m_load, &QPushButton::pressed, this, &FIFOPlayerWindow::LoadRecording);
  connect(m_save, &QPushButton::pressed, this, &FIFOPlayerWindow::SaveRecording);
  connect(m_record, &QPushButton::pressed, this, &FIFOPlayerWindow::StartRecording);
  connect(m_stop, &QPushButton::pressed, this, &FIFOPlayerWindow::StopRecording);
  connect(m_button_box, &QDialogButtonBox::rejected, this, &FIFOPlayerWindow::reject);
  connect(m_early_memory_updates, &QCheckBox::toggled, this,
          &FIFOPlayerWindow::OnEarlyMemoryUpdatesChanged);

  connect(m_frame_range, &SpinBoxRangePair::ValuesChanged, [](int range_start, int range_end) {
    FifoPlayer& player = FifoPlayer::GetInstance();
    player.SetFrameRangeStart(range_start);
    player.SetFrameRangeEnd(range_end);
  });
  connect(m_object_range, &SpinBoxRangePair::ValuesChanged, [](int range_start, int range_end) {
    FifoPlayer& player = FifoPlayer::GetInstance();
    player.SetObjectRangeStart(range_start);
    player.SetObjectRangeEnd(range_end);
  });
}

void FIFOPlayerWindow::LoadRecording()
{
  QString path = QFileDialog::getOpenFileName(this, tr("Open FIFO log"), QString(),
                                              tr("Dolphin FIFO Log (*.dff)"));

  if (path.isEmpty())
    return;

  emit LoadFIFORequested(path);
}

void FIFOPlayerWindow::SaveRecording()
{
  QString path = QFileDialog::getSaveFileName(this, tr("Save FIFO log"), QString(),
                                              tr("Dolphin FIFO Log (*.dff)"));

  if (path.isEmpty())
    return;

  FifoDataFile* file = FifoRecorder::GetInstance().GetRecordedFile();

  bool result = file->Save(path.toStdString());

  if (!result)
    QMessageBox::critical(this, tr("Error"), tr("Failed to save FIFO log."));
}

void FIFOPlayerWindow::StartRecording()
{
  // Start recording
  FifoRecorder::GetInstance().StartRecording(
      m_frame_record_count->value(), [this] { QueueOnObject(this, &FIFOPlayerWindow::Update); });
  Update();
}

void FIFOPlayerWindow::StopRecording()
{
  FifoRecorder::GetInstance().StopRecording();
  Update();
}

void FIFOPlayerWindow::Update()
{
  bool is_running = Core::IsRunning();
  bool is_recording = FifoRecorder::GetInstance().IsRecording();
  bool is_recording_done = FifoRecorder::GetInstance().IsRecordingDone();
  bool is_playing = FifoPlayer::GetInstance().IsPlaying();

  m_frame_range->SetEnabled(is_playing);
  m_object_range->SetEnabled(is_playing);

  m_early_memory_updates->setEnabled(is_playing);

  bool enable_frame_record_count = !is_playing && !is_recording;

  m_frame_record_count_label->setEnabled(enable_frame_record_count);
  m_frame_record_count->setEnabled(enable_frame_record_count);

  m_load->setEnabled(!is_running);
  m_record->setEnabled(is_running && !is_playing);

  m_stop->setVisible(is_running && is_recording);
  m_record->setVisible(!m_stop->isVisible());

  m_save->setEnabled(is_recording_done);

  m_info_label->setText([&] {
    if (is_playing)
    {
      FifoDataFile* file = FifoPlayer::GetInstance().GetFile();
      return tr("%1 frame(s)\n%2 object(s)\nCurrent Frame: %3")
          .arg(QString::number(file->GetFrameCount()),
               QString::number(FifoPlayer::GetInstance().GetFrameObjectCount()),
               QString::number(FifoPlayer::GetInstance().GetCurrentFrameNum()));
    }

    if (is_recording_done)
    {
      FifoDataFile* file = FifoRecorder::GetInstance().GetRecordedFile();
      return tr("%1 FIFO bytes\n%2 memory bytes\n%3 frames")
          .arg(QString::number(file->GetFifoDataBytes()),
               QString::number(file->GetMemoryUpdatesBytes()),
               QString::number(file->GetFrameCount()));
    }

    if (is_running && is_recording)
      return tr("Recording...");

    return tr("No file loaded / recorded.");
  }());
}

void FIFOPlayerWindow::OnFIFOLoaded()
{
  FifoDataFile* file = FifoPlayer::GetInstance().GetFile();

  auto object_count = FifoPlayer::GetInstance().GetFrameObjectCount();
  auto frame_count = file->GetFrameCount();

  m_frame_range->SetMaximumRangeEnd(frame_count);
  m_object_range->SetMaximumRangeEnd(object_count);

  m_frame_range->SetRangeEnd(frame_count);
  m_object_range->SetRangeEnd(object_count);

  Update();
}

void FIFOPlayerWindow::OnEarlyMemoryUpdatesChanged(bool enabled)
{
  FifoPlayer::GetInstance().SetEarlyMemoryUpdates(enabled);
}
