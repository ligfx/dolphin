// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include <QComboBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QTabWidget>
#include <QVBoxLayout>

#include "DolphinQt2/Config/Mapping/MappingWindow.h"

#include "Common/FileSearch.h"
#include "Common/FileUtil.h"
#include "Common/StringUtil.h"
#include "Core/Core.h"
#include "DolphinQt2/Config/Mapping/GCKeyboardEmu.h"
#include "DolphinQt2/Config/Mapping/GCPadEmu.h"
#include "DolphinQt2/Config/Mapping/Hotkey3D.h"
#include "DolphinQt2/Config/Mapping/HotkeyGeneral.h"
#include "DolphinQt2/Config/Mapping/HotkeyGraphics.h"
#include "DolphinQt2/Config/Mapping/HotkeyStates.h"
#include "DolphinQt2/Config/Mapping/HotkeyTAS.h"
#include "DolphinQt2/Config/Mapping/HotkeyWii.h"
#include "DolphinQt2/Config/Mapping/WiimoteEmuExtension.h"
#include "DolphinQt2/Config/Mapping/WiimoteEmuGeneral.h"
#include "DolphinQt2/Config/Mapping/WiimoteEmuMotionControl.h"
#include "DolphinQt2/Settings.h"
#include "InputCommon/ControllerEmu/ControllerEmu.h"
#include "InputCommon/ControllerInterface/ControllerInterface.h"
#include "InputCommon/ControllerInterface/Device.h"
#include "InputCommon/InputConfig.h"

DevicesBox::DevicesBox(EmulatedControllerModel* model) : QGroupBox(tr("Device")), m_model(model)
{
  // CreateWidgets
  auto* layout = new QHBoxLayout();
  m_combo = new QComboBox();
  m_refresh = new QPushButton(tr("Refresh"));

  m_refresh->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  layout->addWidget(m_combo);
  layout->addWidget(m_refresh);

  setLayout(layout);

  // ConnectWidgets
  connect(m_refresh, &QPushButton::clicked,
          [] { Core::RunAsCPUThread([&] { g_controller_interface.RefreshDevices(); }); });

  connect(m_combo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
          [this] { m_model->SetDevice(m_combo->currentText().toStdString()); });

  connect(&Settings::Instance(), &Settings::DevicesChanged, this, &DevicesBox::Update);
  connect(m_model, &EmulatedControllerModel::DefaultDeviceChanged, this, &DevicesBox::Update);

  Update();
};

void DevicesBox::Update()
{
  const QSignalBlocker blocker(m_combo);
  m_combo->clear();

  const auto default_device = m_model->m_controller->GetDefaultDevice().ToString();
  m_combo->addItem(QString::fromStdString(default_device));
  m_combo->setCurrentIndex(0);

  for (const auto& name : g_controller_interface.GetAllDeviceStrings())
  {
    if (name != default_device)
      m_combo->addItem(QString::fromStdString(name));
  }
}

MappingWindow::MappingWindow(QWidget* parent, Type type, int port_num) : QDialog(parent)
{
  m_model.m_port = port_num;

  setWindowTitle(tr("Port %1").arg(port_num + 1));
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  CreateDevicesLayout();
  CreateProfilesLayout();
  CreateResetLayout();
  CreateMainLayout();
  ConnectWidgets();
  SetMappingType(type);
}

void MappingWindow::CreateDevicesLayout()
{
  m_devices_box = new DevicesBox(&m_model);
}

void MappingWindow::CreateProfilesLayout()
{
  m_profiles_layout = new QHBoxLayout();
  m_profiles_box = new QGroupBox(tr("Profile"));
  m_profiles_combo = new QComboBox();
  m_profiles_load = new QPushButton(tr("Load"));
  m_profiles_save = new QPushButton(tr("Save"));
  m_profiles_delete = new QPushButton(tr("Delete"));

  auto* button_layout = new QHBoxLayout();

  m_profiles_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_profiles_combo->setEditable(true);

  m_profiles_layout->addWidget(m_profiles_combo);
  button_layout->addWidget(m_profiles_load);
  button_layout->addWidget(m_profiles_save);
  button_layout->addWidget(m_profiles_delete);
  m_profiles_layout->addItem(button_layout);

  m_profiles_box->setLayout(m_profiles_layout);
}

void MappingWindow::CreateResetLayout()
{
  m_reset_layout = new QHBoxLayout();
  m_reset_box = new QGroupBox(tr("Reset"));
  m_reset_clear = new QPushButton(tr("Clear"));
  m_reset_default = new QPushButton(tr("Default"));

  m_reset_box->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  m_reset_layout->addWidget(m_reset_default);
  m_reset_layout->addWidget(m_reset_clear);

  m_reset_box->setLayout(m_reset_layout);
}

void MappingWindow::CreateMainLayout()
{
  m_main_layout = new QVBoxLayout();
  m_config_layout = new QHBoxLayout();
  m_tab_widget = new QTabWidget();
  m_button_box = new QDialogButtonBox(QDialogButtonBox::Ok);

  m_config_layout->addWidget(m_devices_box);
  m_config_layout->addWidget(m_reset_box);
  m_config_layout->addWidget(m_profiles_box);

  m_main_layout->addItem(m_config_layout);
  m_main_layout->addWidget(m_tab_widget);
  m_main_layout->addWidget(m_button_box);

  setLayout(m_main_layout);
}

void MappingWindow::ConnectWidgets()
{
  connect(m_reset_clear, &QPushButton::clicked, &m_model, &EmulatedControllerModel::Clear);
  connect(m_reset_default, &QPushButton::clicked, &m_model, &EmulatedControllerModel::LoadDefaults);
  connect(m_profiles_save, &QPushButton::clicked, this, &MappingWindow::OnSaveProfilePressed);
  connect(m_profiles_load, &QPushButton::clicked, this, &MappingWindow::OnLoadProfilePressed);
  connect(m_profiles_delete, &QPushButton::clicked, this, &MappingWindow::OnDeleteProfilePressed);
  connect(m_button_box, &QDialogButtonBox::accepted, this, &MappingWindow::accept);
}

void MappingWindow::OnDeleteProfilePressed()
{
  const QString profile_name = m_profiles_combo->currentText();
  const QString profile_path = m_profiles_combo->currentData().toString();

  if (!File::Exists(profile_path.toStdString()))
  {
    QMessageBox error(this);
    error.setIcon(QMessageBox::Critical);
    error.setText(tr("The profile '%1' does not exist").arg(profile_name));
    error.exec();
    return;
  }

  QMessageBox confirm(this);

  confirm.setIcon(QMessageBox::Warning);
  confirm.setText(tr("Are you sure that you want to delete '%1'?").arg(profile_name));
  confirm.setInformativeText(tr("This cannot be undone!"));
  confirm.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);

  if (confirm.exec() != QMessageBox::Yes)
  {
    return;
  }

  m_profiles_combo->removeItem(m_profiles_combo->currentIndex());

  File::Delete(profile_path.toStdString());

  QMessageBox result(this);
  result.setIcon(QMessageBox::Information);
  result.setText(tr("Successfully deleted '%1'.").arg(profile_name));
}

void MappingWindow::OnLoadProfilePressed()
{
  const QString profile_path = m_profiles_combo->currentData().toString();

  if (m_profiles_combo->currentIndex() == 0)
    return;

  m_model.LoadProfile(profile_path.toStdString());
  // This refreshes devices because the default device changed!
  RefreshDevices();
}

void MappingWindow::OnSaveProfilePressed()
{
  const QString profile_name = m_profiles_combo->currentText();
  const QString profile_path = m_profiles_combo->currentData().toString();

  if (profile_name.isEmpty())
    return;

  m_model.SaveProfile(profile_path.toStdString());

  if (m_profiles_combo->currentIndex() == 0)
  {
    m_profiles_combo->addItem(profile_name, profile_path);
    m_profiles_combo->setCurrentIndex(m_profiles_combo->count() - 1);
  }
}

void MappingWindow::SetMappingType(MappingWindow::Type type)
{
  MappingWidget* widget;

  switch (type)
  {
  case Type::MAPPING_GC_KEYBOARD:
    widget = new GCKeyboardEmu(&m_model);
    AddWidget(tr("GameCube Keyboard"), widget);
    setWindowTitle(tr("GameCube Keyboard at Port %1").arg(m_model.GetPort() + 1));
    break;
  case Type::MAPPING_GC_BONGOS:
  case Type::MAPPING_GC_STEERINGWHEEL:
  case Type::MAPPING_GC_DANCEMAT:
  case Type::MAPPING_GCPAD:
    widget = new GCPadEmu(&m_model);
    setWindowTitle(tr("GameCube Controller at Port %1").arg(m_model.GetPort() + 1));
    AddWidget(tr("GameCube Controller"), widget);
    break;
  case Type::MAPPING_WIIMOTE_EMU:
  case Type::MAPPING_WIIMOTE_HYBRID:
  {
    auto* extension = new WiimoteEmuExtension(&m_model);
    widget = new WiimoteEmuGeneral(&m_model, extension);
    setWindowTitle(tr("Wii Remote %1").arg(m_model.GetPort() + 1));
    AddWidget(tr("General and Options"), widget);
    AddWidget(tr("Motion Controls and IR"), new WiimoteEmuMotionControl(&m_model));
    AddWidget(tr("Extension"), extension);
    break;
  }
  case Type::MAPPING_HOTKEYS:
  {
    widget = new HotkeyGeneral(&m_model);
    AddWidget(tr("General"), widget);
    AddWidget(tr("TAS Tools"), new HotkeyTAS(&m_model));
    AddWidget(tr("Wii and Wii Remote"), new HotkeyWii(&m_model));
    AddWidget(tr("Graphics"), new HotkeyGraphics(&m_model));
    AddWidget(tr("3D"), new Hotkey3D(&m_model));
    AddWidget(tr("Save and Load State"), new HotkeyStates(&m_model));
    setWindowTitle(tr("Hotkey Settings"));
    break;
  }
  default:
    return;
  }

  widget->LoadSettings();

  m_model.SetConfig(widget->GetConfig());
  m_profiles_combo->addItem(QStringLiteral(""));

  const std::string profiles_path =
      File::GetUserPath(D_CONFIG_IDX) + "Profiles/" + m_model.GetConfig()->GetProfileName();
  for (const auto& filename : Common::DoFileSearch({profiles_path}, {".ini"}))
  {
    std::string basename;
    SplitPath(filename, nullptr, &basename, nullptr);
    m_profiles_combo->addItem(QString::fromStdString(basename), QString::fromStdString(filename));
  }

  RefreshDevices();
}

void MappingWindow::AddWidget(const QString& name, QWidget* widget)
{
  m_tab_widget->addTab(widget, name);
}
