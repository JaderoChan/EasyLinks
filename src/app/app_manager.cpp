#include "app_manager.h"

#include <easy_translate.hpp>

#include "auto_run_on_startup.h"
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

    aboutDlg_ = new AboutDialog();

    hotkeyMgr_ = new HotkeyManager(this);
    hotkeyMgr_->setSettings(settings_);

    settingsWgt_ = new SettingsWidget(settings_);

    sti_ = new SystemTrayIcon(this);;
    sti_->setToolTip(EASYTR("SystemTrayIcon.ToolTip"));
    sti_->show();

    connect(settingsWgt_, &SettingsWidget::settingsChanged, this, [=](Settings settings)
    {
        if (settings.language != settings_.language)
        {
            if (!setLanguage(settings.language))
                qDebug() << QString("Failed to set language to %1").arg(
                    languageStringId(settings.language));
            sti_->setToolTip(EASYTR("SystemTrayIcon.ToolTip"));
            sti_->updateText();
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

    connect(sti_, &SystemTrayIcon::settingsActionTriggered, settingsWgt_, &SettingsWidget::showAndActivate);
    connect(sti_, &SystemTrayIcon::aboutActionTriggered, aboutDlg_, &AboutDialog::showAndActivate);
    connect(sti_, &SystemTrayIcon::exitActionTriggered, qApp, &QApplication::quit);
}

AppManager::~AppManager()
{
    delete aboutDlg_;
    delete settingsWgt_;
}
