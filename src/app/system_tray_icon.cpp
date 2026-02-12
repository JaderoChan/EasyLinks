#include "system_tray_icon.h"

#include <qicon.h>

#include <easy_translate.hpp>

#include "config.h"

SystemTrayIcon::SystemTrayIcon(QObject* parent)
    : QSystemTrayIcon(QIcon(":/icons/app.ico"), parent)
{
    menu_.addActions({&settingsAction_, &aboutAction_});
    menu_.addSeparator();
    menu_.addAction(&exitAction_);

    setContextMenu(&menu_);

    connect(this, &QSystemTrayIcon::activated, this, &SystemTrayIcon::onActivated);
    connect(&settingsAction_, &QAction::triggered, this, [=]() { emit settingsActionTriggered(); });
    connect(&aboutAction_, &QAction::triggered, this, [=]() { emit aboutActionTriggered(); });
    connect(&exitAction_, &QAction::triggered, this, [=]() { emit exitActionTriggered(); });

    show();
    updateText();
}

void SystemTrayIcon::updateText()
{
    setToolTip(EASYTR("App.Title"));
    settingsAction_.setText(EASYTR("SystemTrayIcon.Action.Settings"));
    aboutAction_.setText(EASYTR("SystemTrayIcon.Action.About"));
    exitAction_.setText(EASYTR("SystemTrayIcon.Action.Exit"));
}

void SystemTrayIcon::onActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
        case ActivationReason::Trigger: // Fallthrough
        case ActivationReason::Context:
            contextMenu()->popup(QCursor::pos());
            break;
        default:
            break;
    }
}
