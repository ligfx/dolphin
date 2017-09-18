// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "DolphinQt2/Settings/GameCubePane.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "Core/ConfigManager.h"
#include "Core/Core.h"
#include "Core/HW/EXI/EXI.h"
#include "Core/HW/EXI/EXI_Device.h"
#include "DolphinQt2/QtUtils/OldBind.h"
// #include "DolphinQt2/QtUtils/ConnectToSubscribable.h"
#include "DolphinQt2/QtUtils/QueueOnObject.h"

Q_DECLARE_METATYPE(ExpansionInterface::TEXIDevices);

GameCubePane::GameCubePane(QWidget* parent) : QWidget(parent)
{
  CreateLayout();
  ConnectLayout();
}

// static BehaviorSubject<Core::State, bool>
// s_core_not_running(Core::State::Uninitialized, Core::OnStateChanged(),
//                    [](Core::State state) { return state == Core::State::Uninitialized; });
//
// // TODO: actually hook this up
// static BehaviorSubject<Core::State, bool>
// s_netplay_not_running(Core::State::Uninitialized, Core::OnStateChanged(),
//                       [](Core::State state) { return state == Core::State::Uninitialized; });

void GameCubePane::ConnectLayout()
{
  // TODO: disable when IPL isn't available
  BindControlToProperty(m_skip_ipl_checkbox, SConfig::GetInstance().bHLE_BS2);
  // BindControlEnabledToProperty(m_skip_ipl_checkbox, s_core_not_running);

  BindControlToProperty(m_system_language_combo, SConfig::GetInstance().SelectedLanguage);
  // BindControlEnabledToProperty(m_system_language_combo, s_core_not_running);

  BindControlToProperty(m_override_language_checkbox, SConfig::GetInstance().bOverrideGCLanguage);
  // BindControlEnabledToProperty(m_override_language_checkbox, s_core_not_running);

  for (size_t i = 0; i < m_exi_device_combos.size(); i++)
  {
    auto* exi_device_combo = m_exi_device_combos[i];

    // TODO: this is lame. figure out autoconnecting for user data?
    BindControlToProperty(
        exi_device_combo, SConfig::GetInstance().m_EXIDevice[i],
        [=](ExpansionInterface::TEXIDevices config_value) {
          return exi_device_combo->findData(config_value);
        },
        [=](int index) {
          return exi_device_combo->currentData().value<ExpansionInterface::TEXIDevices>();
        });
    // BindControlEnabledToProperty(exi_device_combo, s_netplay_not_running);
  }

  for (size_t i = 0; i < SConfig::GetInstance().m_EXIDevice.size(); ++i)
  {
    ConnectToProperty(SConfig::GetInstance().m_EXIDevice[i], this,
                      [=](ExpansionInterface::TEXIDevices new_type) {
                        // Change plugged device! :D
                        if (Core::IsRunning())
                          ExpansionInterface::ChangeDevice(
                              (i == 1) ? 1 : 0,   // SlotB is on channel 1, slotA and SP1 are on 0
                              new_type,           // The device enum to change to
                              (i == 2) ? 2 : 0);  // SP1 is device 2, slots are device 0
                      });
  }

  for (size_t i = 0; i < m_exi_device_buttons.size(); i++)
  {
    // Gray out the memcard path button if we're not on a memcard or AGP
    BindControlEnabledToProperty(m_exi_device_buttons[i],
                                 [=](ExpansionInterface::TEXIDevices type) {
                                   return type == ExpansionInterface::EXIDEVICE_MEMORYCARD ||
                                          type == ExpansionInterface::EXIDEVICE_AGP ||
                                          type == ExpansionInterface::EXIDEVICE_MIC;
                                 },
                                 SConfig::GetInstance().m_EXIDevice[i]);
  }
}

void GameCubePane::CreateLayout()
{
  auto* main_layout = new QVBoxLayout();
  setLayout(main_layout);

  auto* ipl_group = new QGroupBox(tr("IPL Settings"));
  main_layout->addWidget(ipl_group);
  auto* ipl_layout = new QVBoxLayout();
  ipl_group->setLayout(ipl_layout);

  m_skip_ipl_checkbox = new QCheckBox(tr("Skip BIOS"));
  ipl_layout->addWidget(m_skip_ipl_checkbox);

  auto* system_language_layout = new QHBoxLayout();
  ipl_layout->addLayout(system_language_layout);

  m_system_language_label = new QLabel(tr("System Language:"));
  system_language_layout->addWidget(m_system_language_label);

  m_system_language_combo = new QComboBox();
  m_system_language_combo->setToolTip(tr("Sets the GameCube system language."));
  // XXX: should these strings be defined somewhere else?
  for (const auto& language : {
           tr("English"), tr("German"), tr("French"), tr("Spanish"), tr("Italian"), tr("Dutch"),
       })
    m_system_language_combo->addItem(language);
  system_language_layout->addWidget(m_system_language_combo);
  system_language_layout->addStretch();

  m_override_language_checkbox = new QCheckBox(tr("Override Language on NTSC Games"));
  m_override_language_checkbox->setToolTip(tr("Lets the system language be set to values that "
                                              "games were not designed for. This can allow "
                                              "the use of extra translations for a few games, "
                                              "but can also lead to text display issues."));
  ipl_layout->addWidget(m_override_language_checkbox);

  auto* device_group = new QGroupBox(tr("Device Settings"));
  main_layout->addWidget(device_group);
  auto* device_layout = new QGridLayout();
  device_group->setLayout(device_layout);
  device_layout->setColumnStretch(3, 1);

  m_exi_device_combos[0] = new QComboBox();
  m_exi_device_buttons[0] = new QPushButton(tr("..."));
  device_layout->addWidget(new QLabel(tr("Slot A")), 0, 0);
  device_layout->addWidget(m_exi_device_combos[0], 0, 1);
  device_layout->addWidget(m_exi_device_buttons[0], 0, 2);

  m_exi_device_combos[1] = new QComboBox();
  m_exi_device_buttons[1] = new QPushButton(tr("..."));
  device_layout->addWidget(new QLabel(tr("Slot B")), 1, 0);
  device_layout->addWidget(m_exi_device_combos[1], 1, 1);
  device_layout->addWidget(m_exi_device_buttons[1], 1, 2);

  m_exi_device_combos[2] = new QComboBox();
  m_exi_device_combos[2]->setToolTip(
      tr("Serial Port 1 - This is the port which devices such as the net adapter use."));
  device_layout->addWidget(new QLabel(tr("SP1")), 2, 0);
  device_layout->addWidget(m_exi_device_combos[2], 2, 1);

  for (size_t i = 0; i < m_exi_device_combos.size(); i++)
  {
    auto* exi_device_combo = m_exi_device_combos[i];

    // XXX: should these strings be defined somewhere else?
    exi_device_combo->addItem(tr("<Nothing>"), ExpansionInterface::EXIDEVICE_NONE);
    exi_device_combo->addItem(tr("Dummy"), ExpansionInterface::EXIDEVICE_DUMMY);
    if (i == 0 || i == 1)
    {
      exi_device_combo->addItem(tr("Memory Card"), ExpansionInterface::EXIDEVICE_MEMORYCARD);
      exi_device_combo->addItem(tr("GCI Folder"), ExpansionInterface::EXIDEVICE_MEMORYCARDFOLDER);
      exi_device_combo->addItem(tr("USB Gecko"), ExpansionInterface::EXIDEVICE_GECKO);
      exi_device_combo->addItem(tr("Advance Game Port"), ExpansionInterface::EXIDEVICE_AGP);
      exi_device_combo->addItem(tr("Microphone"), ExpansionInterface::EXIDEVICE_MIC);
    }
    else
    {
      exi_device_combo->addItem(tr("Broadband Adapter"), ExpansionInterface::EXIDEVICE_ETH);
    }
  }

  main_layout->addStretch();
}
