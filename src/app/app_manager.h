#pragma once

#include <qobject.h>

#include "about_dialog.h"
#include "hotkey_manager.h"
#include "settings.h"
#include "settings_widget.h"
#include "system_tray_icon.h"

class AppManager : public QObject
{
    Q_OBJECT

public:
    explicit AppManager(QObject* parent = nullptr);

    void setSettings(const Settings& settings);

protected:
    void showAboutDialog();
    void showSettingsWidget();

private:
    Settings settings_;
    HotkeyManager* hotkeyMgr_ = nullptr;
    SystemTrayIcon* sti_ = nullptr;
};
