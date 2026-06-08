#include "app_manager.h"

#include <qdesktopservices.h>
#include <qdir.h>
#include <qmessagebox.h>

#include <easy_translate.hpp>

#include "language.h"
#include "logo_icon.h"
#include "platforms/auto_run_on_startup.h"
#include "utils/qwidget_utils.h"
#include "directory_select_dialog.h"
#include "filelink/controller.h"

AppManager::AppManager(QObject* parent)
    : QObject(parent)
{
    hotkeyMgr_ = new HotkeyManager(this);
    sti_ = new SystemTrayIcon(this);

    connect(sti_, &SystemTrayIcon::patternLinkActionTriggered, this, &AppManager::showPatternsLinkDialog);
    connect(sti_, &SystemTrayIcon::settingsActionTriggered, this, &AppManager::showSettingsWidget);
    connect(sti_, &SystemTrayIcon::aboutActionTriggered, this, &AppManager::showAboutDialog);
    connect(sti_, &SystemTrayIcon::openLogDirActionTriggered, this, &AppManager::openLogDirectory);
    connect(sti_, &SystemTrayIcon::exitActionTriggered, qApp, &QApplication::quit);

    // 链接操作完成后发送通知。
    connect(hotkeyMgr_, &HotkeyManager::linkCompleted, this, &AppManager::onLinkCompleted);
    connect(hotkeyMgr_, &HotkeyManager::patternLinkTriggered, this, &AppManager::triggerPatternLinkFromDir);

    setSettings(loadSettings());

    qApp->setApplicationDisplayName(EASYTR("App.Title"));
}

void AppManager::setSettings(const Settings& settings)
{
    // Language changed.
    if (settings.language != settings_.language)
    {
        setLanguage(settings.language);
        sti_->updateText();
        qApp->setApplicationDisplayName(EASYTR("App.Title"));
    }

    // Auto run on start up changed.
    if (settings.autoRunOnStartUp != settings_.autoRunOnStartUp)
        setAutoRunOnStartUp(settings.autoRunOnStartUp);

    // Update own settings.
    settings_ = settings;
    // Sync settings in storage.
    saveSettings(settings_);
    // 让HotkeyManager处理潜在的热键更新。
    hotkeyMgr_->setSettings(settings_);
}

void AppManager::showPatternsLinkDialog()
{
    auto selectDirDlg = new DirectorySelectDialog();
    selectDirDlg->setAttribute(Qt::WA_DeleteOnClose);
    showAndActivate(selectDirDlg);

    connect(selectDirDlg, &DirectorySelectDialog::selectFinished, this, [this](QStringList dirs)
    {
        auto controller = new FileLinkController(
            dirs, settings_.patterns, settings_.needReview, settings_.linkConfig, this);
        connect(controller, &FileLinkController::patternLinkFinished, this, &AppManager::onPatternLinkCompleted);
        controller->start();
    });
}

void AppManager::showAboutDialog()
{
    auto dlg = new AboutDialog();
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    showAndActivate(dlg);
}

void AppManager::showSettingsWidget()
{
    auto wgt = new SettingsWidget(settings_);
    wgt->setAttribute(Qt::WA_DeleteOnClose);
    connect(wgt, &SettingsWidget::settingsChanged, this, &AppManager::setSettings);
    showAndActivate(wgt);
}

void AppManager::openLogDirectory()
{
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(APP_LOG_DIRPATH)))
    {
        QMessageBox msgBox(
            QMessageBox::Critical,
            EASYTR("Common.Error"),
            EASYTR("AppManager.MessageBox.Text.FailOpenLogDir")
        );
        msgBox.exec();
    }
}

void AppManager::triggerPatternLinkFromDir(QString dir)
{
    if (dir.isEmpty())
        return;

    auto controller = new FileLinkController(
        QStringList{dir}, settings_.patterns, settings_.needReview, settings_.linkConfig, this);
    connect(controller, &FileLinkController::patternLinkFinished, this, &AppManager::onPatternLinkCompleted);
    controller->start();
}

void AppManager::onLinkCompleted(LinkType lt, const QString& targetDir, const LinkStats& stats)
{
    int successEntries = stats.processedEntries - stats.failedEntries;

    QString linkTypeName(EASYTR(lt == LT_HARDLINK ? "LinkType.Hardlink" : "LinkType.Symlink"));
    QString dirName = QDir(targetDir).isRoot() ? targetDir : QDir(targetDir).dirName();
    QString text;

    if (successEntries == stats.processedEntries)           // All success
        text = QString(EASYTR("Notification.LinkCompleted.SuccessOnly"))
            .arg(linkTypeName)
            .arg(successEntries)
            .arg(dirName);
    else if (stats.failedEntries == stats.processedEntries) // All failed
    {
        text = QString(EASYTR("Notification.LinkCompleted.FailedOnly"))
            .arg(linkTypeName)
            .arg(stats.failedEntries)
            .arg(dirName);
    }
    else
    {
        text = QString(EASYTR("Notification.LinkCompleted.SuccessFailed"))
            .arg(linkTypeName)
            .arg(successEntries)
            .arg(dirName)
            .arg(stats.failedEntries);
    }

    sti_->showMessage(QString(EASYTR("App.Title")), text, getLogoIcon(), 3000);
}

void AppManager::onPatternLinkCompleted(const LinkStats& stats)
{
    int successEntries = stats.processedEntries - stats.failedEntries;
    QString text;

    if (successEntries == stats.processedEntries)           // All success
        text = QString(EASYTR("Notification.PatternLinkCompleted.SuccessOnly")).arg(successEntries);
    else if (stats.failedEntries == stats.processedEntries) // All failed
        text = QString(EASYTR("Notification.PatternLinkCompleted.FailedOnly")).arg(stats.failedEntries);
    else
        text = QString(EASYTR("Notification.PatternLinkCompleted.SuccessFailed"))
            .arg(successEntries)
            .arg(stats.failedEntries);

    sti_->showMessage(QString(EASYTR("App.Title")), text, getLogoIcon(), 3000);
}
