#include "app_manager.h"

#include <easy_translate.hpp>

#include "platforms/auto_run_on_startup.h"
#include "language.h"

AppManager::AppManager(QObject* parent)
    : QObject(parent), settings_(loadSettings())
{
    if (!setLanguage(settings_.language))
        qDebug() << QString("Failed to set language to %1").arg(
            languageStringId(settings_.language));

    if (settings_.autoRunOnStartUp != isAutoRunOnStartUp())
    {
        if (!setAutoRunOnStartUp(settings_.autoRunOnStartUp))
            qDebug() << QString("Failed to %1 auto run on start up").arg(
                settings_.autoRunOnStartUp ? "set" : "unset");
    }

    hotkeyMgr_ = new HotkeyManager(this);
    hotkeyMgr_->setSettings(settings_);

    sti_ = new SystemTrayIcon(this);;

    connect(sti_, &SystemTrayIcon::settingsActionTriggered, this, &AppManager::showSettingsWidget);
    connect(sti_, &SystemTrayIcon::aboutActionTriggered, this, &AppManager::showAboutDialog);
    connect(sti_, &SystemTrayIcon::exitActionTriggered, qApp, &QApplication::quit);

    qApp->setApplicationDisplayName(EASYTR("App.Title"));
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
    connect(wgt, &SettingsWidget::settingsChanged, this, [=](Settings settings)
    {
        if (settings.language != settings_.language)
        {
            if (!setLanguage(settings.language))
                qDebug() << QString("Failed to set language to %1").arg(
                    languageStringId(settings.language));
            sti_->updateText();
            qApp->setApplicationDisplayName(EASYTR("App.Title"));
        }

        if (settings.autoRunOnStartUp != settings_.autoRunOnStartUp)
        {
            if (!setAutoRunOnStartUp(settings.autoRunOnStartUp))
                qDebug() << QString("Failed to %1 auto run on start up").arg(
                    settings.autoRunOnStartUp ? "set" : "unset");
        }

        settings_ = settings;
        saveSettings(settings_);
        hotkeyMgr_->setSettings(settings_);
    });
    wgt->showAndActivate();
}
