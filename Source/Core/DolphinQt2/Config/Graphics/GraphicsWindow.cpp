// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "DolphinQt2/Config/Graphics/GraphicsWindow.h"

#include <QDialogButtonBox>
#include <QEvent>
#include <QGroupBox>
#include <QLabel>
#include <QTabWidget>
#include <QVBoxLayout>

#include "Core/ConfigManager.h"
#include "DolphinQt2/Config/Graphics/AdvancedWidget.h"
#include "DolphinQt2/Config/Graphics/EnhancementsWidget.h"
#include "DolphinQt2/Config/Graphics/GeneralWidget.h"
#include "DolphinQt2/Config/Graphics/HacksWidget.h"
#include "DolphinQt2/Config/Graphics/SoftwareRendererWidget.h"
#include "DolphinQt2/MainWindow.h"
#include "DolphinQt2/Settings.h"

GraphicsWindow::GraphicsWindow(X11Utils::XRRConfiguration* xrr_config, MainWindow* parent)
    : QDialog(parent), m_xrr_config(xrr_config)
{
  CreateMainLayout();
  ConnectWidgets();

  setWindowTitle(tr("Graphics"));
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  OnBackendChanged(Settings::Instance().GetVideoBackend());
}

void GraphicsWindow::CreateMainLayout()
{
  auto* main_layout = new QVBoxLayout();
  auto* description_box = new QGroupBox(tr("Description"));
  auto* description_layout = new QVBoxLayout();
  m_description =
      new QLabel(tr("Move the mouse pointer over an option to display a detailed description."));
  m_tab_widget = new QTabWidget();
  m_button_box = new QDialogButtonBox(QDialogButtonBox::Ok);

  description_box->setLayout(description_layout);
  description_box->setMinimumHeight(205);

  m_description->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_description->setWordWrap(true);
  m_description->setAlignment(Qt::AlignTop | Qt::AlignLeft);

  description_layout->addWidget(m_description);

  main_layout->addWidget(m_tab_widget);
  main_layout->addWidget(description_box);
  main_layout->addWidget(m_button_box);

  m_general_widget = new GeneralWidget(m_xrr_config, this);
  m_enhancements_widget = new EnhancementsWidget(this);
  m_hacks_widget = new HacksWidget(this);
  m_advanced_widget = new AdvancedWidget(this);
  m_software_renderer = new SoftwareRendererWidget(this);

  auto set_description = [this](QWidget* widget, const char* description) {
    SetDescription(widget, description);
  };
  m_general_widget->ForEachDescription(set_description);
  m_enhancements_widget->ForEachDescription(set_description);
  m_hacks_widget->ForEachDescription(set_description);
  m_advanced_widget->ForEachDescription(set_description);
  m_software_renderer->ForEachDescription(set_description);

  connect(&Settings::Instance(), &Settings::VideoBackendChanged, this,
          &GraphicsWindow::OnBackendChanged);

  if (Settings::Instance().GetVideoBackend() != "Software Renderer")
  {
    m_tab_widget->addTab(m_general_widget, tr("General"));
    m_tab_widget->addTab(m_enhancements_widget, tr("Enhancements"));
    m_tab_widget->addTab(m_hacks_widget, tr("Hacks"));
    m_tab_widget->addTab(m_advanced_widget, tr("Advanced"));
  }
  else
  {
    m_tab_widget->addTab(m_software_renderer, tr("Software Renderer"));
  }

  setLayout(main_layout);
}

void GraphicsWindow::ConnectWidgets()
{
  connect(m_button_box, &QDialogButtonBox::accepted, this, &QDialog::accept);
}

void GraphicsWindow::OnBackendChanged(const std::string& backend)
{
  setWindowTitle(tr("Dolphin %1 Graphics Configuration").arg(QString::fromStdString(backend)));
  if (backend == "Software Renderer" && m_tab_widget->count() > 1)
  {
    m_tab_widget->clear();
    m_tab_widget->addTab(m_software_renderer, tr("Software Renderer"));
  }

  if (backend != "Software Renderer" && m_tab_widget->count() == 1)
  {
    m_tab_widget->clear();
    m_tab_widget->addTab(m_general_widget, tr("General"));
    m_tab_widget->addTab(m_enhancements_widget, tr("Enhancements"));
    m_tab_widget->addTab(m_hacks_widget, tr("Hacks"));
    m_tab_widget->addTab(m_advanced_widget, tr("Advanced"));
  }
}

void GraphicsWindow::SetDescription(QWidget* widget, const char* description)
{
  m_widget_descriptions[widget] = description;
  widget->installEventFilter(this);
}

bool GraphicsWindow::eventFilter(QObject* object, QEvent* event)
{
  if (!m_widget_descriptions.contains(object))
    return false;

  if (event->type() == QEvent::Enter)
  {
    m_description->setText(tr(m_widget_descriptions[object]));
    return false;
  }

  if (event->type() == QEvent::Leave)
  {
    m_description->setText(
        tr("Move the mouse pointer over an option to display a detailed description."));
  }

  return false;
}
