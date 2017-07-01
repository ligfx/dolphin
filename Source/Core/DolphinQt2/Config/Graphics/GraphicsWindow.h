// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <QDialog>
#include <QHash>

class AdvancedWidget;
class EnhancementsWidget;
class HacksWidget;
class GeneralWidget;
class MainWindow;
class QLabel;
class QTabWidget;
class QDialogButtonBox;
class SoftwareRendererWidget;

namespace X11Utils
{
class XRRConfiguration;
}

class GraphicsWindow final : public QDialog
{
  Q_OBJECT
public:
  explicit GraphicsWindow(X11Utils::XRRConfiguration* xrr_config, QWidget* parent = nullptr);

  bool eventFilter(QObject* object, QEvent* event) override;
  void SetDescription(QWidget* widget, const char* description);

signals:
  void EmulationStarted();
  void EmulationStopped();

private:
  void CreateMainLayout();
  void ConnectWidgets();
  void OnBackendChanged(const std::string& backend);

  QTabWidget* m_tab_widget;
  QLabel* m_description;
  QDialogButtonBox* m_button_box;

  AdvancedWidget* m_advanced_widget;
  EnhancementsWidget* m_enhancements_widget;
  HacksWidget* m_hacks_widget;
  GeneralWidget* m_general_widget;
  SoftwareRendererWidget* m_software_renderer;

  X11Utils::XRRConfiguration* m_xrr_config;

  QHash<QObject*, const char*> m_widget_descriptions;
};
