#include "auto_run_on_startup.h"

#include <qapplication.h>
#include <qdir.h>
#include <qstring.h>
#include <qsettings.h>

bool isAutoRunOnStartUp()
{
    QSettings settings(
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        QSettings::NativeFormat);
    QString appName = QApplication::applicationName();
    QString appPath = "\"" + QDir::toNativeSeparators(QApplication::applicationFilePath()) + "\"";
    if (settings.contains(appName) && settings.value(appName) == appPath)
        return true;
    return false;
}

bool setAutoRunOnStartUp(bool enable)
{
    QSettings settings(
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        QSettings::NativeFormat);
    QString appName = QApplication::applicationName();
    QString appPath = "\"" + QDir::toNativeSeparators(QApplication::applicationFilePath()) + "\"";
    if (enable)
        settings.setValue(appName, appPath);
    else
        settings.remove(appName);
    settings.sync();
    return settings.status() == QSettings::NoError;
}
