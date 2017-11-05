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
#include "DolphinQt2/Config/Mapping/InputDevicesComboBox.h"
#include "DolphinQt2/Config/Mapping/WiimoteEmuExtension.h"
#include "DolphinQt2/Config/Mapping/WiimoteEmuGeneral.h"
#include "DolphinQt2/Config/Mapping/WiimoteEmuMotionControl.h"
#include "DolphinQt2/Settings.h"
#include "InputCommon/ControllerEmu/ControllerEmu.h"
#include "InputCommon/ControllerInterface/ControllerInterface.h"
#include "InputCommon/ControllerInterface/Device.h"
#include "InputCommon/InputConfig.h"

ProfilesBox::ProfilesBox(EmulatedControllerModel* model) : QGroupBox(tr("Profile")), m_model(model)
{
  // CreateWidgets
  auto* layout = new QHBoxLayout();

  m_combo = new QComboBox();
  m_load = new QPushButton(tr("Load"));
  m_save = new QPushButton(tr("Save"));
  m_delete = new QPushButton(tr("Delete"));

  auto* button_layout = new QHBoxLayout();

  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_combo->setEditable(true);

  layout->addWidget(m_combo);
  button_layout->addWidget(m_load);
  button_layout->addWidget(m_save);
  button_layout->addWidget(m_delete);
  layout->addItem(button_layout);

  setLayout(layout);

  // ConnectWidgets
  connect(m_save, &QPushButton::clicked, this, &ProfilesBox::OnSaveProfilePressed);
  connect(m_load, &QPushButton::clicked, this, &ProfilesBox::OnLoadProfilePressed);
  connect(m_delete, &QPushButton::clicked, this, &ProfilesBox::OnDeleteProfilePressed);

  m_combo->addItem(QStringLiteral(""));

  const std::string profiles_path =
      File::GetUserPath(D_CONFIG_IDX) + "Profiles/" + m_model->GetConfig()->GetProfileName();
  for (const auto& filename : Common::DoFileSearch({profiles_path}, {".ini"}))
  {
    std::string basename;
    SplitPath(filename, nullptr, &basename, nullptr);
    m_combo->addItem(QString::fromStdString(basename), QString::fromStdString(filename));
  }
}

void ProfilesBox::OnSaveProfilePressed()
{
  const QString profile_name = m_combo->currentText();
  const QString profile_path = m_combo->currentData().toString();

  if (profile_name.isEmpty())
    return;

  m_model->SaveProfile(profile_path.toStdString());

  if (m_combo->currentIndex() == 0)
  {
    m_combo->addItem(profile_name, profile_path);
    m_combo->setCurrentIndex(m_combo->count() - 1);
  }
}

void ProfilesBox::OnLoadProfilePressed()
{
  const QString profile_path = m_combo->currentData().toString();

  if (m_combo->currentIndex() == 0)
    return;

  m_model->LoadProfile(profile_path.toStdString());
}

void ProfilesBox::OnDeleteProfilePressed()
{
  const QString profile_name = m_combo->currentText();
  const QString profile_path = m_combo->currentData().toString();

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

  m_combo->removeItem(m_combo->currentIndex());

  File::Delete(profile_path.toStdString());

  QMessageBox result(this);
  result.setIcon(QMessageBox::Information);
  result.setText(tr("Successfully deleted '%1'.").arg(profile_name));
}

MappingWindow::MappingWindow(QWidget* parent, Type type, int port_num) : QDialog(parent)
{
  m_model.m_port = port_num;

  setWindowTitle(tr("Port %1").arg(port_num + 1));
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  m_tab_widget = new QTabWidget();
  SetMappingType(type);

  CreateDevicesLayout();
  CreateProfilesLayout();
  CreateResetLayout();
  CreateMainLayout();
  ConnectWidgets();
}

void MappingWindow::CreateDevicesLayout()
{
  auto* devices_layout = new QHBoxLayout();
  m_devices_box = new QGroupBox(tr("Devices"));
  m_devices_combo = new InputDevicesComboBox();
  m_devices_refresh = new QPushButton(tr("Refresh"));

  m_devices_refresh->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  devices_layout->addWidget(m_devices_combo);
  devices_layout->addWidget(m_devices_refresh);

  m_devices_box->setLayout(devices_layout);
}

void MappingWindow::CreateProfilesLayout()
{
  m_profiles_box = new ProfilesBox(&m_model);
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
  connect(m_devices_combo, &InputDevicesComboBox::DeviceChanged, &m_model,
          &EmulatedControllerModel::SetDevice);
  auto update_default_device = [this] {
    m_devices_combo->SetDevice(m_model.GetController()->GetDefaultDevice().ToString());
  };
  connect(&m_model, &EmulatedControllerModel::DefaultDeviceChanged, this, update_default_device);
  update_default_device();

  connect(m_devices_refresh, &QPushButton::clicked,
          [] { Core::RunAsCPUThread([&] { g_controller_interface.RefreshDevices(); }); });
  connect(m_reset_clear, &QPushButton::clicked, &m_model, &EmulatedControllerModel::Clear);
  connect(m_reset_default, &QPushButton::clicked, &m_model, &EmulatedControllerModel::LoadDefaults);
  connect(m_button_box, &QDialogButtonBox::accepted, this, &MappingWindow::accept);
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
}

void MappingWindow::AddWidget(const QString& name, QWidget* widget)
{
  m_tab_widget->addTab(widget, name);
}
