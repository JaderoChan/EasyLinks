#include "app_manager.h"

#include <qdesktopservices.h>
#include <qmessagebox.h>

#include <easy_translate.hpp>

#include "platform/auto_run_on_startup.h"
#include "language.h"

AppManager::AppManager(QObject* parent)
    : QObject(parent)
{
    hotkeyMgr_ = new HotkeyManager(this);
    sti_ = new SystemTrayIcon(this);;

    connect(sti_, &SystemTrayIcon::settingsActionTriggered, this, &AppManager::showSettingsWidget);
    connect(sti_, &SystemTrayIcon::aboutActionTriggered, this, &AppManager::showAboutDialog);
    connect(sti_, &SystemTrayIcon::openLogDirActionTriggered, this, &AppManager::openLogDirectory);
    connect(sti_, &SystemTrayIcon::exitActionTriggered, qApp, &QApplication::quit);

    setSettings(loadSettings());

    qApp->setApplicationDisplayName(EASYTR("App.Title"));
}

void AppManager::setSettings(const Settings& settings)
{
    // Language changed
    if (settings.language != settings_.language)
    {
        if (!setLanguage(settings.language))
            qWarning() << QString("Failed to set language to %1").arg(
                languageStringId(settings.language));

        sti_->updateText();
        qApp->setApplicationDisplayName(EASYTR("App.Title"));
    }

    // Auto run on start up changed
    if (settings.autoRunOnStartUp != settings_.autoRunOnStartUp)
    {
        if (!setAutoRunOnStartUp(settings.autoRunOnStartUp))
            qWarning() << QString("Failed to %1 auto run on start up").arg(
                settings.autoRunOnStartUp ? "set" : "unset");
    }

    // Update own settings
    settings_ = settings;
    // Sync settings in storage
    saveSettings(settings_);
    // 让HotkeyManager处理潜在的热键更新
    hotkeyMgr_->setSettings(settings_);
}

void AppManager::showAboutDialog()
{
    auto dlg = new AboutDialog();
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->showAndActivate();
}

void AppManager::showSettingsWidget()
{
    auto wgt = new SettingsWidget(settings_);
    wgt->setAttribute(Qt::WA_DeleteOnClose);
    connect(wgt, &SettingsWidget::settingsChanged, this, &AppManager::setSettings);
    wgt->showAndActivate();
}

void AppManager::openLogDirectory()
{
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(APP_LOG_DIRPATH)))
    {
        QMessageBox::warning(
            nullptr,
            EASYTR("Common.Error"),
            EASYTR("AppManager.MessageBox.Text.FailOpenLogDir"),
            EASYTR("Common.Ok")
        );
    }
}
