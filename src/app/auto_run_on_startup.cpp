#include "auto_run_on_startup.h"

#include <qapplication.h>
#include <qdir.h>
#include <qstring.h>
#include <qsettings.h>

bool isAutoRunOnStartUp()
{
#ifdef Q_OS_WIN
    QSettings settings(
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        QSettings::NativeFormat);
    QString appName = QCoreApplication::applicationName();
    QString appPath = "\"" + QDir::toNativeSeparators(QCoreApplication::applicationFilePath()) + "\"";
    if (settings.contains(appName) && settings.value(appName) == appPath)
        return true;
    return false;
#else
    // todo
    return false;
#endif
}

bool setAutoRunOnStartUp(bool enable)
{
#ifdef Q_OS_WIN
    QSettings settings(
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        QSettings::NativeFormat);
    QString appName = QCoreApplication::applicationName();
    QString appPath = "\"" + QDir::toNativeSeparators(QCoreApplication::applicationFilePath()) + "\"";
    if (enable)
        settings.setValue(appName, appPath);
    else
        settings.remove(appName);
    settings.sync();
    return settings.status() == QSettings::NoError;
#else
    // todo
    return false;
#endif
}
