#include "system_tray_icon.h"

#include <easy_translate.hpp>

#include "config.h"
#include "utils/logo_icon.h"

SystemTrayIcon::SystemTrayIcon(QObject* parent)
    : QSystemTrayIcon(getLogoIcon(), parent)
{
    menu_.addActions({&settingsAction_, &aboutAction_, &openLogDirAction_});
    menu_.addSeparator();
    menu_.addAction(&exitAction_);

    setContextMenu(&menu_);

    connect(this, &QSystemTrayIcon::activated, this, &SystemTrayIcon::onActivated);
    connect(&settingsAction_, &QAction::triggered, this, [=]() { emit settingsActionTriggered(); });
    connect(&aboutAction_, &QAction::triggered, this, [=]() { emit aboutActionTriggered(); });
    connect(&openLogDirAction_, &QAction::triggered, this, [=]() { emit openLogDirActionTriggered(); });
    connect(&exitAction_, &QAction::triggered, this, [=]() { emit exitActionTriggered(); });

    show();
    updateText();
}

void SystemTrayIcon::updateText()
{
    setToolTip(EASYTR("App.Title"));
    settingsAction_.setText(EASYTR("SystemTrayIcon.Action.Settings"));
    aboutAction_.setText(EASYTR("SystemTrayIcon.Action.About"));
    openLogDirAction_.setText(EASYTR("SystemTrayIcon.Action.OpenLogDir"));
    exitAction_.setText(EASYTR("SystemTrayIcon.Action.Exit"));
}

void SystemTrayIcon::onActivated(QSystemTrayIcon::ActivationReason reason)
{
#ifndef Q_OS_MAC
    switch (reason)
    {
        case ActivationReason::Context:
        case ActivationReason::Trigger:
            contextMenu()->popup(QCursor::pos());
            break;
        default:
            break;
    }
#endif // !Q_OS_MAC
}
